// vim: noet ts=4 sw=4 sts=0
#include <Elementary.h>
#include <string.h>
#include "ui.h"




static Tree * root;
static const char * filename;




<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
static win      :* win;
static toolbar  :* toolbar;
static genlist  :* tree;
static scroller :* props;
static Elm_Genlist_Item_Class * ic;




static const char * number_lookup[] = {
	"无", "一", "二", "三", "四", "五", "六", "七", "八", "九", "十", "多"
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
	{ _____ :+ toolbar(win); toolbar = _____; }
	toolbar :  hint_align(::HINT_FILL, 0);
	box     :  pack_end(toolbar);
	toolbar :  show;

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

	toolbar: item_append(NULL, "新建", ANY on_toolbar_new , NULL);
	toolbar: item_append(NULL, "打开", ANY on_toolbar_open, NULL);
	toolbar: item_append(NULL, "保存", ANY on_toolbar_save, NULL);
	toolbar: item_append(NULL, "退出", ANY elm_exit       , NULL);

	//------------------- tree
	{
		// frame
		frame :+ frame(win);
		frame :  text("家谱");
		frame :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
		frame :  hint_align (::HINT_FILL  , ::HINT_FILL  );
		box   :  pack_end(frame);
		frame :  show;

		// genlist
		{ __ :+ genlist(win); tree = __; }
		tree :  tree_effect_enabled(true);
		tree :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
		tree :  hint_align (::HINT_FILL  , ::HINT_FILL  );
		frame:  content(tree);
		tree :  show;
		item := genlist_item;
	}

	// init item class
	ic = elm_genlist_item_class_new();
	ic->item_style = "tree_effect";
	{
		const char * text_get(Tree * t)
		{
			static char str[64];
			snprintf(str, sizeof(str), "%s %s：%s子女",
					(t->gender ? "女" : "男"),
					t->name,
					lookup(pack_length(t->childs)));
			return strdup(str);
		}

		Evas_Object * content_get(Tree * t, SKIP, const char * part)
		{
			if (!strcmp(part, "elm.swallow.icon")) {
				btn :+ button(win);
				btn :  text("生");
				void on_click(Tree * t)
				{
					$_(child, tree_new(t));
					$_(item, tree::selected_item);
					if (item::expanded)
						tree: item_append(ic, child, item, ELM_GENLIST_ITEM_TREE, (void *)&edit, child);
					item: expanded(true);
					tree: realized_items_update;
				}
				btn :- clicked(ANY on_click, t);
				return btn;
			}
			else {
				btn :+ button(win);
				btn :  text("删除");
				void on_click(Tree * t)
				{
					$_(item, tree::selected_item);
					$_(parent, item::parent);
					parent := genlist_item;

					tree_free(t);
					item: item_del;

					tree : realized_items_update;
					props: content(NULL);

					if (!parent) {
						root = tree_new(NULL);
						tree: item_append(ic, root, NULL, ELM_GENLIST_ITEM_TREE, (void *)&edit, root);
						return;
					}

					Tree * pa = parent::item_data;
					if (!pack_length(pa->childs))
						parent: expanded(false);
				}
				btn :- clicked(ANY on_click, t);
				return btn;
			}
		}

		ic->func.text_get    = ANY text_get;
		ic->func.content_get = ANY content_get;
	}

	{
		// bind "expand" and "contract" related events
		void on_expand_request(SKIP, SKIP, Elm_Object_Item * item)
		{
			Tree * t = item::item_data;
			if (pack_length(t->childs))
				item: expanded(true);
			else popup_message("请生！");
		}

		void on_expanded(SKIP, SKIP, Elm_Object_Item * item)
		{
			Tree * t = item::item_data;
			pack_walk(t->childs, Tree, child, {
				tree: item_append(ic, child, item, ELM_GENLIST_ITEM_TREE, (void *)&edit, child);
			});
		}

		void on_contract_request(SKIP, SKIP, Elm_Object_Item * item)
		{
			item: expanded(false);
		}

		void on_contracted(SKIP, SKIP, Elm_Object_Item * item)
		{
			item: subitems_clear;
		}

		tree :- expand,request  (ANY on_expand_request  , NULL);
		tree :- expanded        (ANY on_expanded        , NULL);
		tree :- contract,request(ANY on_contract_request, NULL);
		tree :- contracted      (ANY on_contracted      , NULL);
	}

	//------------------- properties
	{
		// frame
		frame :+ frame(win);
		frame :  text("信息");
		frame :  hint_weight(::HINT_EXPAND,   0        );
		frame :  hint_align (::HINT_FILL  , ::HINT_FILL);
		box   :  pack_end(frame);
		frame :  show;

		// scroller, just add some 3d-inside-look
		{ ___ :+ scroller(win); props = ___; }
		props :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
		props :  hint_align (::HINT_FILL  , ::HINT_FILL  );
		props :  show;
		props :  bounce(false, false);
		table :* p =  limit_min_size(props, 100, 50);
		p     :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
		p     :  hint_align (::HINT_FILL  , ::HINT_FILL  );
		frame :  content(p);
		p     :  show;
	}

	//------------------- done!
	win: show;

	document_new(tree_new(NULL));
	extern const char * filename_to_open;	// in main.c
	if (filename_to_open) document_open(filename_to_open);

	elm_run();
	elm_shutdown();

	return 0;
}


static Evas_Object * limit_min_size(object :* o, int w, int h)
{
	table :+ table(win);
	$_(rect, evas_object_rectangle_add(evas_object_evas_get(o)));
	rect := object;
	rect :  hint_min(w, h);
	rect :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
	rect :  hint_align (::HINT_FILL  , ::HINT_FILL  );
	table:  pack(rect, 0, 0, 1, 1);
	table:  pack(o   , 0, 0, 1, 1);
	return table;
}


static void popup_message(const char * message)
{
	popup :+ popup(win);
	popup :  orient(::ORIENT_BOTTOM);
	popup :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
	popup :  part_text("title,text", message);
	void on_block_clicked(void * data, popup :* popup)
	{
		popup: del;
	}
	popup :- block,clicked(ANY on_block_clicked, NULL);
	popup :  show;
}


static void popup_file_selector(const char * title, bool is_save,
								void done(const char * filename))
{
	popup :+ popup(win);
	popup :  orient(::ORIENT_TOP);
	popup :  hint_weight(::HINT_EXPAND, ::HINT_EXPAND);
	popup :  part_text("title,text", title);
	void on_block_clicked(void * data, popup :* popup)
	{
		popup: del;
	}
	popup :- block,clicked(ANY on_block_clicked, NULL);

	fs :+ fileselector(popup);
	fs :  is_save(is_save);
	fs :  expandable(false);
	fs :  buttons_ok_cancel(false);
	fs :  path(getenv("PWD"));
	fs :  hint_align(::HINT_FILL, ::HINT_FILL);
	fs :  show;
	popup:content(limit_min_size(fs, 400, 400));

	btn :+ button(popup);
	btn :  text("ok");
	popup: part_content("button1", btn);
	popup: data("cb.done", done);
	popup: data("cb.fs"  , fs  );
	void on_btn_clicked(popup :* popup)
	{
		void (*done_cb)(const char * fn) = popup::data("cb.done");
		fileselector :* f = popup::data("cb.fs");
		done_cb(f::selected);
		popup: del;
	}
	btn :- clicked(ANY on_btn_clicked, popup);

	popup: show;
}


static void toolbar_no_selected()
{
	$_(tbitem, toolbar::selected_item);
	tbitem := toolbar_item;
	if (tbitem)
		tbitem: selected(false);
}


static void document_new(Tree * t)
{
	tree: clear;
	if (root) tree_free(root);

	root = t;

	tree: item_append(ic, root, NULL, ELM_GENLIST_ITEM_TREE, (void *)&edit, root);
	props:content(NULL);
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
	static int pos = 0;

	table :+ table(win);
	table :  hint_weight(::HINT_EXPAND,   0        );
	table :  hint_align (::HINT_FILL  , ::HINT_FILL);

	inline void edit_done(object :* ob, double weight_x)
	{
		ob := object;
		ob :  hint_weight(  weight_x , ::HINT_EXPAND);
		ob :  hint_align (::HINT_FILL, ::HINT_FILL  );
		ob :  hint_padding(5, 5, 5, 0);
		table:pack(ob, pos++, 0, 1, 1);
		ob :  show;
	}

	inline void edit_text(const char ** var, double weight_x)
	{
		ob :+ entry(win);
		ob :  scrollable(true);
		ob :  single_line(true);
		ob :  text(*var);

		void on_changed(const char ** var, entry :* ob)
		{
			if (*var) free(ANY *var);
			*var = strdup(ob::text);
			tree: realized_items_update;
		}
		ob :- changed(ANY on_changed, var);

		edit_done(ANY ob, weight_x);
	}

	inline void edit_bool(int * var, double weight_x,
			const char * label_on, const char * label_off)
	{
		ob :+ check(win);
		ob :  style("toggle");
		ob :  part_text("on" , label_on );
		ob :  part_text("off", label_off);
		ob :  state(*var);

		void on_changed(int * var, check :* ob)
		{
			*var = ob::state;
			tree: realized_items_update;
		}
		ob :- changed(ANY on_changed, var);

		edit_done(ANY ob, weight_x);
	}


	edit_bool(&t->gender, 0  , "女", "男");
	edit_text(&t->name  , 0.3);
	edit_text(&t->note  , 1  );


	props: content(table);
	table: show;
}
>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

