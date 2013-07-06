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
#define ANY (void *)




<<<<<<
static win      :* win;
static toolbar  :* toolbar;
static genlist  :* tree;
static scroller :* props;
static Elm_Genlist_Item_Class * ic;
>>>>>>


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
<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//------------------- main window
	{ __ :+ win(NULL, "tramee", ::BASIC); win = __; }
	win :  title("tramee");
	win :  resize(800, 600);
	win :- delete,request(ANY elm_exit, NULL);

	bg  :+ bg(win);
	bg  :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
	win :  resize_object(bg);
	bg  :  show;

	//------------------- main vbox
	box :+ box(win);
	box :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
	win :  resize_object(box);
	box :  show;

	//------------------- toolbar in main vbox
	toolbar :+ toolbar(win);
	toolbar :  hint_align(::HINT_FILL, 0);
	box     :  pack_end(toolbar);
	toolbar :  show;
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	// toolbar items
	void on_toolbar_new()
	{
		toolbar_no_selected();
		if (filename) free(ANY filename);
		filename = NULL;
		document_new(tree_new(NULL));
	}
	void on_toolbar_open()
	{
		toolbar_no_selected();
		popup_file_selector("打开什么？", false, &document_open);
	}
	void on_toolbar_save()
	{
		toolbar_no_selected();
		if (filename) document_save(filename);
		else popup_file_selector("保存到哪？", true, &document_save);
	}

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	toolbar :  item_append(NULL, "新建", ANY on_toolbar_new , NULL);
	toolbar :  item_append(NULL, "打开", ANY on_toolbar_open, NULL);
	toolbar :  item_append(NULL, "保存", ANY on_toolbar_save, NULL);
	toolbar :  item_append(NULL, "退出", ANY elm_exit       , NULL);

	//------------------- panes: in main vbox
	panes :+ panes(win);
	panes :  horizontal(true);
	panes :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
	panes :  hint_align (::HINT_FILL  , ::HINT_FILL  );
	win   :  resize_object(panes);
	box   :  pack_end(panes);
	panes :  show;

	//------------------- tree: in pane's top
	{
		// frame
		frame :+ frame(win);
		frame :  text("家谱树");
		frame :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
		frame :  hint_align (::HINT_FILL  , ::HINT_FILL  );
		panes :  part_content_set("left", frame);
		frame :  show;

		// genlist
		{ __ :+ genlist(win); tree = __; }
		tree :  tree_effect_enabled(true);
		tree :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
		tree :  hint_align (::HINT_FILL  , ::HINT_FILL  );
		frame:  content(tree);
		tree :  show;
	}
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// init item class
	ic = elm_genlist_item_class_new();
	ic->item_style = "tree_effect";

	ic->func.text_get = (void *)$(const char *, (Tree * t) {
		static char str[64];
		snprintf(str, 32, "%s：%s子%s女", t->name,
				lookup(pack_length(t->boys)), lookup(t->ngirl));
		return strdup(str);
	});

<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	ic->func.content_get =
		(void *)$(Evas_Object *, (Tree * t, void * $2, const char * part) {
			if (!strcmp(part, "elm.swallow.icon")) {
				btn :+ button(win);
				btn :  text("生子");
				$$$$(btn, "clicked", $(void, (Tree * t) {
					$_(boy, tree_new(t));
					$_(item, tree::selected_item);
					item := genlist_item;
					if (item::expanded)
						tree: item_append(ic, boy, item, ELM_GENLIST_ITEM_TREE, (void *)&edit, boy);
					item: expanded(true);
					tree: realized_items_update;
				}), t);
				return btn;
			}
			else {
				btn :+ button(win);
				btn :  text("删除");
				$$$$(btn, "clicked", $(void, (Tree * t) {
					$_(item, tree::selected_item);
					item := genlist_item;
					$_(parent, item::parent);
					parent := genlist_item;

					tree_free(t);
					item: item_del;
					//elm_object_item_del(item);

					tree : realized_items_update;
					props: content(NULL);

					if (!parent) {
						root = tree_new(NULL);
						tree: item_append(ic, root, NULL, ELM_GENLIST_ITEM_TREE, (void *)&edit, root);
						return;
					}

					Tree * pa = parent::item_data;
					if (!pack_length(pa->boys))
						parent: expanded(false);
				}), t);
				return btn;
			}
		});
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

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

#define EDIT_INIT($label, $x, $y) ({ \
	$_(label, elm_label_add(win)); \
	elm_object_scale_set(label, 1.2); \
	evas_object_size_hint_padding_set(label, 5, 0, 0, 0); \
	elm_object_text_set(label, "<font color=#48c>" $label "</>"); \
	elm_table_pack(table, label, ($x)<<1, ($y), 1, 1); \
	evas_object_show(label); \
})
#define EDIT_DONE($x, $y) ({ \
	evas_object_size_hint_weight_set(ob, \
			EVAS_HINT_EXPAND, EVAS_HINT_EXPAND); \
	evas_object_size_hint_fill_set(ob, \
			EVAS_HINT_FILL, EVAS_HINT_FILL); \
	evas_object_size_hint_padding_set(ob, 5, 5, 5, 0); \
	elm_table_pack(table, ob, ($x)<<1 | 1, ($y), 1, 1); \
	evas_object_show(ob); \
})

#define EDIT_TEXT($label, $x, $y, $var) ({ \
	EDIT_INIT($label, $x, $y); \
	$_(ob, elm_entry_add(win)); \
	elm_entry_single_line_set(ob, EINA_TRUE); \
	elm_object_text_set(ob, ($var)); \
	EDIT_DONE($x, $y); \
 \
	$$$$(ob, "changed", $(void, (const char ** s, Evas_Object * ob) { \
		if (*s) free((void *)*s); \
		*s = strdup(elm_object_text_get(ob)); \
		elm_genlist_realized_items_update(tree); \
	}), &($var)); \
})
#define EDIT_UINT($label, $x, $y, $var) ({ \
	EDIT_INIT($label, $x, $y); \
	$_(ob, elm_slider_add(win)); \
	elm_slider_min_max_set(ob, 0, 6); \
	elm_slider_value_set(ob, ($var)); \
	EDIT_DONE($x, $y); \
 \
	$$$$(ob, "changed", $(void, (size_t * s, Evas_Object * ob) { \
		*s = round(elm_slider_value_get(ob)); \
		elm_genlist_realized_items_update(tree); \
	}), &($var)); \
	$$$$(ob, "slider,drag,stop", $(void, (void * $1, Evas_Object * ob) { \
		elm_slider_value_set(ob, round(elm_slider_value_get(ob))); \
	}), &($var)); \
 \
	char * format(double num) \
	{ \
		return (char *)lookup(round(num)); \
	} \
	elm_slider_indicator_format_function_set(ob, &format, NULL); \
	elm_slider_units_format_function_set(ob, &format, NULL); \
	elm_slider_unit_format_set(ob, ""); \
})

	//-------------- EDIT HERE ----------------
	EDIT_TEXT("姓名", 0, 0, t->name);
	EDIT_TEXT("妻子", 1, 0, t->wife);
	EDIT_UINT("女数", 2, 0, t->ngirl);
	//-------------- EDIT DONE ----------------

	elm_object_content_set(props, table);
	evas_object_show(table);
}

