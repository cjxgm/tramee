// vim: noet ts=4 sw=4 sts=0
#include <Elementary.h>
#include <string.h>
#include "ui.h"




Tree * root;
const char * filename;




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


static const char * number_lookup[] = {
	"无", "一", "二", "三", "四", "五", "多"
};
static inline const char * lookup(int num)
{
	if (num >= array_length(number_lookup)-1)
		return number_lookup[array_length(number_lookup)-1];
	return number_lookup[num];
}




EAPI_MAIN int elm_main(int argc, char * argv[]);

static Evas_Object * limit_min_size(Evas_Object * o, int w, int h);
static void popup_message(const char * message);
static void popup_file_selector(const char * title, bool is_save,
								void done(const char * filename));
static void toolbar_no_selected();

static void document_new(Tree * t);
static void document_open(const char * fn);
static void document_save(const char * fn);
static void edit(Tree * t);




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
	elm_toolbar_item_append(toolbar, NULL, "新建",
		(void *)$(void, () {
			toolbar_no_selected();
			if (filename) free((void *)filename);
			filename = NULL;
			document_new(tree_new(NULL));
		}), NULL);

	elm_toolbar_item_append(toolbar, NULL, "打开",
		(void *)$(void, () {
			toolbar_no_selected();
			popup_file_selector("打开什么？", false, &document_open);
		}), NULL);

	elm_toolbar_item_append(toolbar, NULL, "保存",
		(void *)$(void, () {
			toolbar_no_selected();
			if (filename) document_save(filename);
			else popup_file_selector("保存到哪？", true, &document_save);
		}), NULL);

	elm_toolbar_item_append(toolbar, NULL, "退出", (void *)&elm_exit, NULL);

	//------------------- panes: in main vbox
	$_(panes, elm_panes_add(win));
	elm_panes_horizontal_set(panes, EINA_TRUE);
	evas_object_size_hint_weight_set(panes,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panes,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_win_resize_object_add(win, panes);
	elm_box_pack_end(box, panes);
	evas_object_show(panes);

	//------------------- tree: in pane's top
	// frame
	$_(tree_frame, elm_frame_add(win));
	elm_object_text_set(tree_frame, "家谱树");
	evas_object_size_hint_weight_set(tree_frame,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(tree_frame,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(panes, "left", tree_frame);
	evas_object_show(tree_frame);

	// genlist
	tree = elm_genlist_add(win);
	elm_genlist_tree_effect_enabled_set(tree, EINA_TRUE);
	evas_object_size_hint_weight_set(tree,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(tree,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(tree_frame, tree);
	evas_object_show(tree);

	// init item class
	ic = elm_genlist_item_class_new();
	ic->item_style = "tree_effect";

	ic->func.text_get = (void *)$(const char *, (Tree * t) {
		static char str[64];
		snprintf(str, 32, "%s：%s子%s女", t->name,
				lookup(pack_length(t->boys)), lookup(t->ngirl));
		return strdup(str);
	});

	ic->func.content_get =
		(void *)$(Evas_Object *, (Tree * t, void * $2, const char * part) {
			if (!strcmp(part, "elm.swallow.icon")) {
				$_(btn, elm_button_add(win));
				elm_object_text_set(btn, "生子");
				$$$$(btn, "clicked", $(void, (Tree * t) {
					$_(boy, tree_new(t));
					$_(item, elm_genlist_selected_item_get(tree));
					if (elm_genlist_item_expanded_get(item))
						elm_genlist_item_append(tree, ic, boy, item,
								ELM_GENLIST_ITEM_TREE, (void *)&edit, boy);
					elm_genlist_item_expanded_set(item, EINA_TRUE);
					elm_genlist_realized_items_update(tree);
				}), t);
				return btn;
			}
			else {
				$_(btn, elm_button_add(win));
				elm_object_text_set(btn, "删除");
				$$$$(btn, "clicked", $(void, (Tree * t) {
					$_(item, elm_genlist_selected_item_get(tree));
					$_(parent, elm_genlist_item_parent_get(item));

					tree_free(t);
					elm_object_item_del(item);

					elm_genlist_realized_items_update(tree);
					elm_object_content_set(props, NULL);

					if (!parent) {
						root = tree_new(NULL);
						elm_genlist_item_append(tree, ic, root, NULL,
								ELM_GENLIST_ITEM_TREE, (void *)&edit, root);
						return;
					}

					Tree * pa = elm_object_item_data_get(parent);
					if (!pack_length(pa->boys))
						elm_genlist_item_expanded_set(parent, EINA_FALSE);
				}), t);
				return btn;
			}
		});

	// init genlist for expand
	$$$$(tree, "expand,request",
		$(void, (void * $1, void * $2, Elm_Object_Item * item) {
			Tree * t = elm_object_item_data_get(item);
			if (pack_length(t->boys))
				elm_genlist_item_expanded_set(item, EINA_TRUE);
			else popup_message("请生子！");
		}), NULL);
	$$$$(tree, "expanded",
		$(void, (void * $1, void * $2, Elm_Object_Item * item) {
			Tree * t = elm_object_item_data_get(item);
			pack_walk(t->boys, Tree, boy, {
				elm_genlist_item_append(tree, ic, boy, item,
						ELM_GENLIST_ITEM_TREE, (void *)&edit, boy);
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

	//------------------- properties: in pane's bottom
	// frame
	$_(props_frame, elm_frame_add(win));
	elm_object_text_set(props_frame, "属性");
	evas_object_size_hint_weight_set(props_frame,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(props_frame,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(panes, "right", props_frame);
	evas_object_show(props_frame);

	// scroller
	props = elm_scroller_add(win);
	evas_object_size_hint_weight_set(props,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(props,
			EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(props_frame, props);
	evas_object_show(props);

	//------------------- done!
	evas_object_show(win);

	if (filename) document_open(filename);
	else document_new(tree_new(NULL));

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
	evas_object_size_hint_weight_set(popup,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
	evas_object_size_hint_weight_set(popup,
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
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
		void (*done_cb)(const char * fn) =
				evas_object_data_get(o, "cb:done");
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


static void document_new(Tree * t)
{
	elm_genlist_clear(tree);
	if (root) tree_free(root);

	root = t;

	elm_genlist_item_append(tree, ic, root, NULL,
			ELM_GENLIST_ITEM_TREE, (void *)&edit, root);
	elm_object_content_set(props, NULL);
}


static void document_open(const char * fn)
{
	if (!fn) {
		popup_message("你他妈到底想打开什么？！！");
		return;
	}

	Tree * t = tree_load(fn);
	if (!t) {
		popup_message("打开你妹啊！");
		return;
	}
	document_new(t);

	if (filename != fn) {
		if (filename) free((void *)filename);
		filename = strdup(fn);
	}

	popup_message("打开完毕。");
}


static void document_save(const char * fn)
{
	if (!fn) {
		popup_message("你他妈到底想保存什么？！！");
		return;
	}

	if (tree_save(root, fn)) {
		popup_message("保存你妹啊！");
		return;
	}
	if (filename != fn) {
		if (filename) free((void *)filename);
		filename = strdup(fn);
	}

	popup_message("保存完毕。");
}


static void edit(Tree * t)
{
	$_(table, elm_table_add(win));
	evas_object_size_hint_weight_set(table,
			EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_fill_set(table,
			EVAS_HINT_FILL, EVAS_HINT_FILL);

	int i = 0;

#define EDIT_INIT($label) ({ \
	$_(label, elm_label_add(win)); \
	elm_object_scale_set(label, 1.2); \
	evas_object_size_hint_padding_set(label, 5, 0, 0, 0); \
	elm_object_text_set(label, $label "："); \
	elm_table_pack(table, label, 0, i, 1, 1); \
	evas_object_show(label); \
})
#define EDIT_DONE() ({ \
	evas_object_size_hint_weight_set(ob, \
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); \
	evas_object_size_hint_fill_set(ob, \
			EVAS_HINT_FILL, EVAS_HINT_FILL); \
	evas_object_size_hint_padding_set(ob, 5, 5, 5, 0); \
	elm_table_pack(table, ob, 1, i, 1, 1); \
	evas_object_show(ob); \
	i++; \
})

#define EDIT_TEXT($label, $var) ({ \
	EDIT_INIT($label); \
	$_(ob, elm_entry_add(win)); \
	elm_entry_single_line_set(ob, EINA_TRUE); \
	elm_object_text_set(ob, ($var)); \
	EDIT_DONE(); \
 \
	$$$$(ob, "changed", $(void, (const char ** s, Evas_Object * ob) { \
		if (*s) free((void *)*s); \
		*s = strdup(elm_object_text_get(ob)); \
		elm_genlist_realized_items_update(tree); \
	}), &($var)); \
})
#define EDIT_UINT($label, $var) ({ \
	EDIT_INIT($label); \
	$_(ob, elm_spinner_add(win)); \
	elm_spinner_min_max_set(ob, 0, 9); \
	elm_spinner_round_set(ob, 1); \
	elm_spinner_step_set(ob, 1); \
	elm_spinner_label_format_set(ob, "%0.0g"); \
	elm_spinner_value_set(ob, ($var)); \
	EDIT_DONE(); \
 \
	$$$$(ob, "changed", $(void, (size_t * s, Evas_Object * ob) { \
		*s = elm_spinner_value_get(ob); \
		elm_genlist_realized_items_update(tree); \
	}), &($var)); \
})

	//-------------- EDIT HERE ----------------
	EDIT_TEXT("姓名", t->name);
	EDIT_TEXT("妻子", t->wife);
	EDIT_UINT("女儿数", t->ngirl);
	//-------------- EDIT DONE ----------------

	elm_object_content_set(props, table);
	evas_object_show(table);
}

