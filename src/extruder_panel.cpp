#include "extruder_panel.h"
#include "state.h"
#include "spdlog/spdlog.h"

#include <limits>

LV_IMG_DECLARE(back);
LV_IMG_DECLARE(spoolman_img);
LV_IMG_DECLARE(extrude_img);
LV_IMG_DECLARE(retract_img);
LV_IMG_DECLARE(extruder);

ExtruderPanel::ExtruderPanel(KWebSocketClient &websocket_client,
			     std::mutex &lock,
			     Numpad &numpad,
			     SpoolmanPanel &sm)
  : NotifyConsumer(lock)
  , ws(websocket_client)
  , panel_cont(lv_obj_create(lv_scr_act()))
  , spoolman_panel(sm)
  , extruder_temp(ws, panel_cont, &extruder, 150,
	  "Extruder", lv_palette_main(LV_PALETTE_RED), false, numpad, "extruder", NULL, NULL)
  , temp_selector(panel_cont, "Extruder Temperature (C)",
		  {"180", "190", "200", "210", "220", "230", "240", ""},
		  std::numeric_limits<uint32_t>::max(), &ExtruderPanel::_handle_callback, this)
  , length_selector(panel_cont, "Extrude Length (mm)",
		    {"5", "10", "15", "20", "25", "30", "35", ""}, 1, &ExtruderPanel::_handle_callback, this)
  , speed_selector(panel_cont, "Extrude Speed (mm/s)",
		   {"1", "2", "5", "10", "25", "35", "50", ""}, 2, &ExtruderPanel::_handle_callback, this)
  , spoolman_btn(panel_cont, &spoolman_img, "Spoolman", &ExtruderPanel::_handle_callback, this)
  , extrude_btn(panel_cont, &extrude_img, "Extrude", &ExtruderPanel::_handle_callback, this)
  , retract_btn(panel_cont, &retract_img, "Retract", &ExtruderPanel::_handle_callback, this)
  , back_btn(panel_cont, &back, "Back", &ExtruderPanel::_handle_callback, this)    
{
  lv_obj_move_background(panel_cont);
  lv_obj_clear_flag(panel_cont, LV_OBJ_FLAG_SCROLLABLE);  
  lv_obj_set_size(panel_cont, LV_PCT(100), LV_PCT(100));

  spoolman_btn.disable();  

  static lv_coord_t grid_main_row_dsc[] = {LV_GRID_FR(3), LV_GRID_FR(6), LV_GRID_FR(6), LV_GRID_FR(6),
    LV_GRID_TEMPLATE_LAST};
  static lv_coord_t grid_main_col_dsc[] = {LV_GRID_FR(7), LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
  
  lv_obj_clear_flag(panel_cont, LV_OBJ_FLAG_SCROLLABLE);
  
  lv_obj_set_grid_dsc_array(panel_cont, grid_main_col_dsc, grid_main_row_dsc);
  lv_obj_add_flag(extruder_temp.get_sensor(), LV_OBJ_FLAG_FLOATING);
  lv_obj_align(extruder_temp.get_sensor(), LV_ALIGN_TOP_LEFT, 60, 0);

  // lv_obj_set_size(extruder_temp.get_sensor(), 350, 60);
  // col 0
  // lv_obj_set_grid_cell(extruder_temp.get_sensor(), LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_grid_cell(speed_selector.get_label(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);
  lv_obj_set_grid_cell(speed_selector.get_selector(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_set_grid_cell(length_selector.get_label(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
  lv_obj_set_grid_cell(length_selector.get_selector(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_set_grid_cell(temp_selector.get_label(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 3, 1);  
  lv_obj_set_grid_cell(temp_selector.get_selector(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 3, 1);
  
  // col 2
  lv_obj_set_grid_cell(spoolman_btn.get_container(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 0, 2);
  lv_obj_set_grid_cell(retract_btn.get_container(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_END, 0, 2);
  lv_obj_set_grid_cell(extrude_btn.get_container(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_START, 2, 2);
  lv_obj_set_grid_cell(back_btn.get_container(), LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_END, 2, 2);

  ws.register_notify_update(this);    
}

ExtruderPanel::~ExtruderPanel() {
  if (panel_cont != NULL) {
    lv_obj_del(panel_cont);
    panel_cont = NULL;
  }
}

void ExtruderPanel::foreground() {
  auto v = State::get_instance()
    ->get_data("/printer_state/extruder/temperature"_json_pointer);
  if (!v.is_null()) {
    int value = v.template get<int>();

    if (value >= 170) {
      extrude_btn.enable();
      retract_btn.enable();
    } else {
      extrude_btn.disable();
      retract_btn.disable();
    }
  }

  lv_obj_move_foreground(panel_cont);
}

void ExtruderPanel::enable_spoolman() {
  spoolman_btn.enable();
}

void ExtruderPanel::consume(json& j) {
  std::lock_guard<std::mutex> lock(lv_lock);
  auto temp_value = j["/params/0/extruder/temperature"_json_pointer];
  if (!temp_value.is_null()) {
    int value = temp_value.template get<int>();
    extruder_temp.update_value(value);

    if (value >= 170) {
      extrude_btn.enable();
      retract_btn.enable();
    } else {
      extrude_btn.disable();
      retract_btn.disable();
    }
  }
}

void ExtruderPanel::handle_callback(lv_event_t *e) {
  spdlog::trace("handling extruder panel callback");
  if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t *selector = lv_event_get_target(e);
    uint32_t idx = lv_btnmatrix_get_selected_btn(selector);
    const char * v = lv_btnmatrix_get_btn_text(selector, idx);

    if (selector == temp_selector.get_selector()) {
      ws.gcode_script(fmt::format("SET_HEATER_TEMPERATURE HEATER=extruder TARGET={}", v));
      temp_selector.set_selected_idx(idx);
    }

    if (selector == length_selector.get_selector()) {
      length_selector.set_selected_idx(idx);
    }

    if (selector == speed_selector.get_selector()) {
      speed_selector.set_selected_idx(idx);
    }

    spdlog::trace("selector {} {} {}, {} {} {}", fmt::ptr(selector), idx, v,
		  fmt::ptr(temp_selector.get_selector()),
		  fmt::ptr(length_selector.get_selector()),
		  fmt::ptr(speed_selector.get_selector()));
    
  } else if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
    lv_obj_t *btn = lv_event_get_target(e);

    if (btn == back_btn.get_button()) {
      lv_obj_move_background(panel_cont);
    }

    if (btn == extrude_btn.get_button()) {
      const char * len = lv_btnmatrix_get_btn_text(length_selector.get_selector(),
						   length_selector.get_selected_idx());
      const char *speed = lv_btnmatrix_get_btn_text(speed_selector.get_selector(),
						    speed_selector.get_selected_idx());
      ws.gcode_script(fmt::format("M83\nG1 E{} F{}", len, std::stoi(speed) * 60));
    }

    if (btn == retract_btn.get_button()) {
      const char * len = lv_btnmatrix_get_btn_text(length_selector.get_selector(),
						   length_selector.get_selected_idx());
      const char *speed = lv_btnmatrix_get_btn_text(speed_selector.get_selector(),
						    speed_selector.get_selected_idx());
      ws.gcode_script(fmt::format("M83\nG1 E-{} F{}", len, std::stoi(speed) * 60));
    }

    if (btn == spoolman_btn.get_button()) {
      spoolman_panel.foreground();
    }
  }
}
