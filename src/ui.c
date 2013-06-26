// vim: noet ts=4 sw=4 sts=0
#include <Elementary.h>
#include <string.h>
#include "ui.h"


Tree * root;


// simpler callback add
#define $$$$($object, $event, $callback, $arg) \
	evas_object_smart_callback_add($object, $event, \
			(void *)($callback), (void *)($arg))
#define $$$($object, $event, $callback, $arg) \
	evas_object_event_callback_add($object, $event, \
			(void *)($callback), (void *)($arg))



static Evas_Object * win;
static Evas_Object * toolbar;
static Evas_Object * tree;
static Evas_Object * props;
static Elm_Genlist_Item_Class * ic;
#if 0
static Evas_Object * stack;
static Evas_Object * menu_node;
static Elm_Object_Item * menu_add;
static Elm_Object_Item * menu_delete;
static Evas_Object * props_current;

#endif

EAPI_MAIN int elm_main(int argc, char * argv[]);

static Evas_Object * limit_min_size(Evas_Object * o, int w, int h);
static void popup_message(const char * message);
static void popup_file_selector(const char * title, bool is_save,
								void done(const char * filename));
static void toolbar_no_selected();

#if 0
static void document_new();
static void document_open(const char * fn);
static void document_save(const char * fn);
static Operator * op_find_by_name(const char * name);
static void op_add(Operator * op, float * values);

#endif

void ui_run()
{
	ELM_MAIN()
	main(0, NULL);
}

EAPI_MAIN int elm_main(int argc, char * argv[])
{
	//------------------- main window
	win = elm_win_util_standard_add("ipu", "Image Proc Unit");
	evas_object_resize(win, 800, 600);
	$$$$(win, "delete,request", &elm_exit, NULL);

	//------------------- main vbox
	$_(box, elm_box_add(win));
	evas_object_size_hint_weight_set(box,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, box);
	evas_object_show(box);

	//------------------- toolbar: in main vbox
	toolbar = elm_toolbar_add(win);
	evas_object_size_hint_align_set(toolbar, EVAS_HINT_FILL, 0);
	elm_box_pack_end(box, toolbar);
	evas_object_show(toolbar);

	// toolbar items
	elm_toolbar_item_append(toolbar, "document-new", "新建",
		(void *)$(void, () {
			toolbar_no_selected();
		}), NULL);
	elm_toolbar_item_append(toolbar, "document-open", "打开",
		(void *)$(void, () {
			toolbar_no_selected();
			popup_file_selector("打开什么？", false, NULL);
		}), NULL);
	elm_toolbar_item_append(toolbar, "document-save", "保存",
		(void *)$(void, () {
			toolbar_no_selected();
			popup_file_selector("保存到哪？", true, NULL);
		}), NULL);
	elm_toolbar_item_append(toolbar, "edit-delete", "退出", (void *)&elm_exit, NULL);

	//------------------- panes: in main vbox
	$_(panes, elm_panes_add(win));
	elm_panes_horizontal_set(panes, EINA_TRUE);
	evas_object_size_hint_weight_set(panes,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, panes);
	//elm_box_pack_end(box, panes);
	evas_object_show(panes);

	//------------------- tree: in pane's top
	// frame
	$_(tree_frame, elm_frame_add(win));
	elm_object_text_set(tree_frame, "家谱树");
	evas_object_size_hint_weight_set(tree_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(tree_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(panes, "top", tree_frame);
	evas_object_show(tree_frame);

	// genlist
	tree = elm_genlist_add(win);
	elm_genlist_tree_effect_enabled_set(tree, EINA_TRUE);
	evas_object_size_hint_weight_set(tree, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(tree, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(tree_frame, tree);
	evas_object_show(tree);

	// init item class
	ic = elm_genlist_item_class_new();
	ic->item_style = "tree_effect";
	ic->func.text_get = (void *)$(const char *, (Tree * t) {
		return strdup(t->name);
	});
	ic->func.content_get = NULL;
	ic->func.state_get = NULL;
	ic->func.del = NULL;	// TODO

	// init genlist for expand
	$$$$(tree, "expand,request",
		$(void, (void * $1, void * $2, Elm_Object_Item * item) {
			elm_genlist_item_expanded_set(item, EINA_TRUE);
		}), NULL);
	$$$$(tree, "expanded",
		$(void, (void * $1, void * $2, Elm_Object_Item * item) {
			Tree * t = elm_object_item_data_get(item);
			pack_walk(t->boys, Tree, boy, {
				elm_genlist_item_append(tree, ic, boy, item,
						ELM_GENLIST_ITEM_TREE, NULL, NULL);
			});
		}), NULL);

	// init genlist for contract
	$$$$(tree, "contract,request",
		$(void, (void * $1, void * $2, Elm_Object_Item * item) {
			elm_genlist_item_expanded_set(item, EINA_FALSE);
		}), NULL);
	$$$$(tree, "contracted",
		$(void, (void * $1, void * $2, Elm_Object_Item * item) {
			elm_genlist_item_subitems_clear(item);
		}), NULL);

	// add root item
	elm_genlist_item_append(tree, ic, root, NULL, ELM_GENLIST_ITEM_TREE, NULL, NULL);

	//------------------- properties: in pane's bottom
	// frame
	$_(props_frame, elm_frame_add(win));
	elm_object_text_set(props_frame, "属性");
	evas_object_size_hint_weight_set(props_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(props_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(panes, "bottom", props_frame);
	evas_object_show(props_frame);

	// scroller
	props = elm_scroller_add(win);
	evas_object_size_hint_weight_set(props, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(props, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(props_frame, props);
	evas_object_show(props);
#if 0

	// menu
	menu_node = elm_menu_add(win);
	// show menu when right clicked
	$$$(nodes, EVAS_CALLBACK_MOUSE_DOWN,
		$(void, (void * $1, void * $2, void * $3, Evas_Event_Mouse_Down * ev) {
			if (ev->button == 3) {
				// when no item selected, disable "Delete" item
				elm_object_item_disabled_set(menu_delete, !elm_list_selected_item_get(nodes));

				elm_menu_move(menu_node, ev->canvas.x, ev->canvas.y);
				evas_object_show(menu_node);
			}
		}), NULL);

	menu_add = elm_menu_item_add(menu_node, NULL, "document-new", "Add", NULL, NULL);

	menu_delete = elm_menu_item_add(menu_node, NULL,
		"edit-delete", "Delete", (void *)$(void, () {
			$_(item, elm_list_selected_item_get(nodes));
			$_(o, elm_menu_item_object_get(item));
			Operator * op = evas_object_data_get(o, "ipu:operator");

			elm_object_content_unset(props);
			evas_object_hide(op->table);
			props_current = NULL;

			free(evas_object_data_get(o, "ipu:v"));
			elm_object_item_del(item);

			execute_nodes();
		}), NULL);

	//------------------- properties
	// frame
	$_(props_frame, elm_frame_add(win));
	elm_object_text_set(props_frame, "Properties");
	evas_object_size_hint_weight_set(props_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(props_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(content, props_frame);
	evas_object_show(props_frame);

	// scroller
	props = elm_scroller_add(win);
	evas_object_size_hint_weight_set(props, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(props, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(props_frame, props);
	evas_object_show(props);

	//------------------- image stack
	// frame
	$_(stack_frame, elm_frame_add(win));
	elm_object_text_set(stack_frame, "Image Stack");
	evas_object_size_hint_weight_set(stack_frame, 0.75, 1);
	evas_object_size_hint_align_set(stack_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(content, stack_frame);
	evas_object_show(stack_frame);

	// scroller
	$_(stack_scroller, elm_scroller_add(win));
	evas_object_size_hint_weight_set(stack_scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(stack_scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(stack_frame, stack_scroller);
	evas_object_show(stack_scroller);

	// table
	stack = elm_table_add(win);
	evas_object_size_hint_weight_set(stack, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_fill_set(stack, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(stack_scroller, stack);
	evas_object_show(stack);

	//------------------- prepare operators
	ops_register_operators();
#endif
	//------------------- done!
	evas_object_show(win);

	elm_run();
	elm_shutdown();

	return 0;
}

static Evas_Object * limit_min_size(Evas_Object * o, int w, int h)
{
	$_(table, elm_table_add(win));
	$_(rect, evas_object_rectangle_add(evas_object_evas_get(o)));
	evas_object_size_hint_min_set(rect, w, h);
	elm_table_pack(table, rect, 0, 0, 1, 1);
	elm_table_pack(table, o, 0, 0, 1, 1);
	return table;
}

static void popup_message(const char * message)
{
	$_(popup, elm_popup_add(win));
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_BOTTOM);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", message);
	evas_object_show(popup);

	$$$$(popup, "block,clicked", $(void, (void * $1, Evas_Object * o) {
		evas_object_del(o);
	}), NULL);
}

static void popup_file_selector(const char * title, bool is_save,
								void done(const char * filename))
{
	$_(popup, elm_popup_add(win));
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_TOP);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_text_set(popup, "title,text", title);
	$$$$(popup, "block,clicked", $(void, (void * $1, Evas_Object * o) {
		evas_object_del(o);
	}), NULL);

	$_(fs, elm_fileselector_add(popup));
	elm_fileselector_is_save_set(fs, is_save);
	elm_fileselector_expandable_set(fs, false);
	elm_fileselector_buttons_ok_cancel_set(fs, false);
	elm_fileselector_path_set(fs, getenv("PWD"));
	evas_object_size_hint_align_set(fs, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(fs);
	elm_object_content_set(popup, limit_min_size(fs, 400, 400));

	$_(btn, elm_button_add(popup));
	elm_object_text_set(btn, "OK");
	elm_object_part_content_set(popup, "button1", btn);

	// set data and bind event
	evas_object_data_set(popup, "cb:done", done);
	evas_object_data_set(popup, "cb:fs", fs);
	$$$$(btn, "clicked", $(void, (Evas_Object * o) {
		void (*done_cb)(const char * fn) = evas_object_data_get(o, "cb:done");
		Evas_Object * f = evas_object_data_get(o, "cb:fs");
		done_cb(elm_fileselector_selected_get(f));
		evas_object_del(o);
	}), popup);

	evas_object_show(popup);
}

static void toolbar_no_selected()
{
	$_(item, elm_toolbar_selected_item_get(toolbar));
	if (item) elm_toolbar_item_selected_set(item, false);
}

#if 0
static void document_new()
{
	if (props_current) {
		elm_object_content_unset(props);
		evas_object_hide(props_current);
	}
	elm_table_clear(stack, true);

	Elm_Object_Item * item;
	while ((item = elm_list_last_item_get(nodes))) {
		$_(o, elm_menu_item_object_get(item));
		free(evas_object_data_get(o, "ipu:v"));
		elm_object_item_del(item);
	}
	elm_list_go(nodes);
}

static void document_open(const char * fn)
{
	if (!fn) {
		popup_message("So, what on earth are you fucking to open?!!");
		return;
	}

	FILE * fp = fopen(fn, "r");
	if (!fp) {
		popup_message("Cannot open that.");
		return;
	}

	document_new();

	char op_name[64];
	while (fscanf(fp, "%63s", op_name) != EOF) {
		$_(op, op_find_by_name(op_name));
		if (!op) {
			fclose(fp);
			popup_message("Invalid file!");
			return;
		}

		float * values = new(float, * op->nprop);
		for (int i=0; i<op->nprop; i++)
			fscanf(fp, "%g", &values[i]);
		op_add(op, values);
	}

	fclose(fp);
	popup_message("Opened!");

	elm_list_go(nodes);
	elm_list_item_bring_in(elm_list_last_item_get(nodes));
	execute_nodes();
}

static void document_save(const char * fn)
{
	if (!fn) {
		popup_message("So, why you clicked the fucking save?!!");
		return;
	}

	FILE * fp = fopen(fn, "w");
	if (!fp) {
		popup_message("Cannot save to there.");
		return;
	}

	$_(item, elm_list_first_item_get(nodes));
	while (item) {
		$_(o, elm_menu_item_object_get(item));
		Operator * op  = evas_object_data_get(o, "ipu:operator");
		float * values = evas_object_data_get(o, "ipu:v");

		fprintf(fp, "%s", op->name);
		for (int i=0; i<op->nprop; i++)
			fprintf(fp, " %g", values[i]);
		fprintf(fp, "\n");

		item = elm_list_item_next(item);
	}

	fclose(fp);
	popup_message("Saved!");
}

static Operator * op_find_by_name(const char * name)
{
	for (int i=0; i<ops_used; i++)
		if (!strcmp(name, ops[i].name))
			return &ops[i];
	return NULL;
}

// values can be NULL if you want to use default value's copy
static void op_add(Operator * op, float * values)
{
	void node_select_cb()
	{
		$_(o, elm_menu_item_object_get(elm_list_selected_item_get(nodes)));
		Operator * op = evas_object_data_get(o, "ipu:operator");

		// switch properties editing widgets to the selected's
		if (props_current) {
			elm_object_content_unset(props);
			evas_object_hide(props_current);
		}
		props_current = op->table;
		elm_object_content_set(props, props_current);

		// set properties
		float * values = evas_object_data_get(o, "ipu:v");
		for (int i=0; i<op->nprop; i++)
			elm_spinner_value_set(op->objs[i], values[i]);

		// then, show it
		evas_object_show(props_current);

		execute_nodes();
	};

	$_(o, elm_menu_item_object_get(elm_list_item_append(nodes,
			op->name, NULL, NULL, (void *)&node_select_cb, NULL)));

	// setup datas needed
	// operator
	evas_object_data_set(o, "ipu:operator", op);
	// values
	if (!values) {
		values = new(float, * op->nprop);
		for (int i=0; i<op->nprop; i++)
			values[i] = op->infos[i].value;
	}
	evas_object_data_set(o, "ipu:v", values);
}
#endif
