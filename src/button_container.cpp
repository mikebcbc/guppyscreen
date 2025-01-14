#include "button_container.h"
#include "spdlog/spdlog.h"
#include <string>

ButtonContainer::ButtonContainer(lv_obj_t *parent,
				 const void *btn_img,
				 const char *text,
				 lv_event_cb_t cb,
				 void* user_data)
  : btn_cont(lv_obj_create(parent))
  , btn(lv_imgbtn_create(btn_cont))
  , label(lv_label_create(btn_cont))
{
    lv_obj_set_style_pad_all(btn_cont, 0, 0);
    lv_obj_set_size(btn_cont, 140, 90);

    lv_obj_clear_flag(btn_cont, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_add_style(btn_cont, &style, LV_PART_MAIN);
    lv_imgbtn_set_src(btn, LV_IMGBTN_STATE_RELEASED, NULL, btn_img, NULL);
    
    lv_obj_set_width(btn, LV_SIZE_CONTENT);
    // lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, 0);

    if (cb != NULL) {
      // lv_obj_add_event_cb(btn_cont, cb, LV_EVENT_CLICKED, user_data);
      lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user_data);
    }

    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_palette_darken(LV_PALETTE_GREY, 1), LV_STATE_DISABLED);

    lv_obj_align_to(label, btn, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    // lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 5);

}

ButtonContainer::~ButtonContainer() {
}

lv_obj_t *ButtonContainer::get_container() {
  return btn_cont;
}

lv_obj_t *ButtonContainer::get_button() {
  return btn;
}

void ButtonContainer::disable() {
  lv_obj_add_state(btn, LV_STATE_DISABLED);
  lv_obj_add_state(label, LV_STATE_DISABLED);
  
}

void ButtonContainer::enable() {
  lv_obj_clear_state(btn, LV_STATE_DISABLED);
  lv_obj_clear_state(label, LV_STATE_DISABLED);
}
