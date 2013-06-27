// experiment for panes
//------------------- panes: in main vbox
$_(panes, elm_panes_add(win));
elm_panes_horizontal_set(panes, EINA_TRUE);
evas_object_size_hint_weight_set(panes,
		EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
elm_win_resize_object_add(win, panes);
evas_object_show(panes);

$_(tree_frame, elm_frame_add(win));
elm_object_text_set(tree_frame, "家谱树");
evas_object_size_hint_weight_set(tree_frame, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
evas_object_size_hint_align_set(tree_frame, EVAS_HINT_FILL, EVAS_HINT_FILL);
elm_object_part_content_set(panes, "left", tree_frame);
evas_object_show(tree_frame);

