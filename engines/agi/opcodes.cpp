/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

AgiInstruction *logicNamesTest;
AgiInstruction *logicNamesCmd;

AgiInstruction insV1Test[] = {
	{ "",					"",			&cond_unknown },		// 00
	{ "equaln",				"vn",		&cond_equal },			// 01
	{ "equalv",				"vv",		&cond_equalv },			// 02
	{ "lessn",				"vn",		&cond_less },			// 03
	{ "lessv",				"vv",		&cond_lessv },			// 04
	{ "greatern",			"vn",		&cond_greater },		// 05
	{ "greaterv",			"vv",		&cond_greaterv },		// 06
	{ "isset",				"v",		&cond_isset_v1 },		// 07
	{ "has",				"n",		&cond_has },			// 08
	{ "said",				"nnnn",		&cond_said2 },			// 09
	{ "posn",				"nnnnn",	&cond_posn },			// 0A
	{ "controller",			"n",		&cond_controller },		// 0B
	{ "obj.in.room",		"nv",		&cond_obj_in_room },	// 0C
	{ "said",				"nnnnnn",	&cond_said3 },			// 0D
	{ "have.key",			"",			&cond_have_key },		// 0E
	{ "said",				"nn",		&cond_said1 },			// 0F
	{ "bit",				"nv",		&cond_unknown },		// 10
};

AgiInstruction insV1[] = {
	{ "return",				"",			NULL },						// 00
	{ "increment",			"v",		&cmd_increment },			// 01
	{ "decrement",			"v",		&cmd_decrement },			// 02
	{ "assignn",			"vn",		&cmd_assignn },				// 03
	{ "assignv",			"vv",		&cmd_assignv },				// 04
	{ "addn",				"vn",		&cmd_addn },				// 05
	{ "addv",				"vv",		&cmd_addv },				// 06
	{ "subn",				"vn",		&cmd_subn },				// 07
	{ "subv",				"vv",		&cmd_subv },				// 08
	{ "load.view",			"n",		&cmd_load_view },			// 09
	{ "animate.obj",		"n",		&cmd_animate_obj },			// 0A
	{ "new.room",			"n",		&cmd_new_room },			// 0B
	{ "draw.pic",			"v",		&cmd_draw_pic },			// 0C
	{ "print",				"s",		&cmd_print },				// 0D
	{ "status",				"",			&cmd_status },				// 0E
	{ "save.game",			"",			&cmd_save_game },			// 0F
	{ "restore.game",		"",			&cmd_load_game },			// 10
	{ "...",				"",			&cmd_init_disk },			// 11 TODO
	{ "restart.game",		"",			&cmd_restart_game },		// 12
	{ "random",				"v",		&cmd_random },				// 13 TODO: 1 vs 3 vars
	{ "get",				"n",		&cmd_get },					// 14
	{ "drop",				"n",		&cmd_drop },				// 15
	{ "draw",				"n",		&cmd_draw },				// 16
	{ "erase",				"n",		&cmd_erase },				// 17
	{ "position",			"nnn",		&cmd_position },			// 18
	{ "position.v",			"nvv",		&cmd_position_f },			// 19
	{ "get.posn",			"nvv",		&cmd_get_posn },			// 1A
	{ "set.cel",			"nn",		&cmd_set_cel },				// 1B
	{ "set.loop",			"nn",		&cmd_set_loop },			// 1C
	{ "end.of.loop",		"nn",		&cmd_end_of_loop },			// 1D
	{ "reverse.loop",		"nn",		&cmd_reverse_loop },		// 1E
	{ "move.obj",			"nnnnn",	&cmd_move_obj },			// 1F
	{ "set.view",			"nn",		&cmd_set_view },			// 20
	{ "follow.ego",			"nnn",		&cmd_follow_ego },			// 21
	{ "...",				"",			&cmd_unknown },				// 22
	{ "...",				"",			&cmd_unknown },				// 23
	{ "ignore.blocks",		"n",		&cmd_ignore_blocks },		// 24
	{ "observe.blocks",		"n",		&cmd_observe_blocks },		// 25
	{ "wander",				"n",		&cmd_wander },				// 26
	{ "reposition",			"nvv",		&cmd_reposition },			// 27
	{ "stop.motion",		"n",		&cmd_stop_motion },			// 28
	{ "start.motion",		"n",		&cmd_start_motion },		// 29
	{ "stop.cycling",		"n",		&cmd_stop_cycling },		// 2A
	{ "start.cycling",		"n",		&cmd_start_cycling },		// 2B
	{ "stop.update",		"n",		&cmd_stop_update },			// 2C
	{ "start.update",		"n",		&cmd_start_update },		// 2D
	{ "program.control",	"",			&cmd_program_control },		// 2E
	{ "player.control",		"",			&cmd_player_control },		// 2F
	{ "set.priority",		"nn",		&cmd_set_priority },		// 30
	{ "release.priority",	"n",		&cmd_release_priority },	// 31
	{ "add.to.pic",			"nnnnnn",	&cmd_add_to_pic },			// 32 TODO: 7 vs 8 args
	{ "set.horizon",		"n",		&cmd_set_horizon },			// 33
	{ "ignore.horizon",		"n",		&cmd_ignore_horizon },		// 34
	{ "observe.horizon",	"n",		&cmd_observe_horizon },		// 35
	{ "load.logics",		"n",		&cmd_load_logic },			// 36
	{ "object.on.water",	"n",		&cmd_object_on_water },		// 37
	{ "load.pic",			"v",		&cmd_load_pic },			// 38
	{ "load.sound",			"n",		&cmd_load_sound },			// 39
	{ "sound",				"nn",		&cmd_sound },				// 3A
	{ "stop.sound",			"",			&cmd_stop_sound },			// 3B
	{ "set.v",				"v",		&cmd_set_v },				// 3C
	{ "reset.v",			"v",		&cmd_reset_v },				// 3D
	{ "toggle.v",			"v",		&cmd_toggle_v },			// 3E
	{ "new.room.v",			"v",		&cmd_new_room_f },			// 3F
	{ "call",				"n",		&cmd_call },				// 40
	{ "...",				"",			&cmd_unknown },				// 41
	{ "set.speed",			"v",		&cmd_set_speed },			// 42
	{ "move.obj.v",			"nvvvv",	&cmd_move_obj_f },			// 43
	{ "...",				"",			&cmd_unknown },				// 44
	{ "...",				"",			&cmd_unknown },				// 45
	{ "...",				"",			&cmd_unknown },				// 46
	{ "...",				"",			&cmd_unknown },				// 47
	{ "...",				"nv",		&cmd_unknown },				// 48 get.priority??
	{ "ignore.objs",		"n",		&cmd_ignore_objs },			// 49
	{ "observe.objs",		"n",		&cmd_observe_objs },		// 4A
	{ "distance",			"nnv",		&cmd_distance },			// 4B
	{ "object.on.land",		"n",		&cmd_object_on_land },		// 4C
	{ "...",				"nv",		&cmd_unknown },				// 4D set.priority.v???
	{ "...",				"",			&cmd_unknown },				// 4E
	{ "load.logics",		"n",		&cmd_load_logic },			// 4F TODO: what is the other load.logics then?
	{ "display",			"nnns",		&cmd_display },				// 50 TODO: 4 vs 3 args
	{ "prevent.input???",	"",			&cmd_unknown },				// 51
	{ "...",				"",			&cmd_unknown },				// 52
	{ "...",				"n",		&cmd_unknown },				// 53 ???
	{ "...",				"",			&cmd_unknown },				// 54 ???
	{ "stop.motion",		"",			&cmd_stop_motion },			// 55 or force.update??
	{ "discard.view",		"n",		&cmd_discard_view },		// 56
	{ "discard.pic",		"v",		&cmd_discard_pic },			// 57
	{ "...",				"nn",		&cmd_unknown },				// 58
	{ "...",				"",			&cmd_unknown },				// 59
	{ "last.cel",			"nv",		&cmd_last_cel },			// 5A
	{ "set.cel.v",			"nv",		&cmd_set_cel_f },			// 5B
	{ "...",				"",			&cmd_unknown },				// 5C
	{ "load.view",			"n",		&cmd_load_view },			// 5D
	{ "...",				"",			&cmd_unknown },				// 5E
	{ "...",				"",			&cmd_unknown },				// 5F
	{ "setbit",				"nv",		&cmd_unknown },				// 60
};

AgiInstruction insV2Test[] = {
	{ "",					"",			&cond_unknown },			// 00
	{ "equaln",				"vn",		&cond_equal },				// 01
	{ "equalv",				"vv",		&cond_equalv },				// 02
	{ "lessn",				"vn",		&cond_less },				// 03
	{ "lessv",				"vv",		&cond_lessv },				// 04
	{ "greatern",			"vn",		&cond_greater },			// 05
	{ "greaterv",			"vv",		&cond_greaterv },			// 06
	{ "isset",				"n",		&cond_isset },				// 07
	{ "issetv",				"v",		&cond_issetv },				// 08
	{ "has",				"n",		&cond_has },				// 09
	{ "obj.in.room",		"nv",		&cond_obj_in_room},			// 0A
	{ "posn",				"nnnnn",	&cond_posn },				// 0B
	{ "controller",			"n",		&cond_controller },			// 0C
	{ "have.key",			"",			&cond_have_key},			// 0D
	{ "said",				"",			&cond_said },				// 0E
	{ "compare.strings",	"ss",		&cond_compare_strings },	// 0F
	{ "obj.in.box",			"nnnnn",	&cond_obj_in_box },			// 10
	{ "center.posn",		"nnnnn",	&cond_center_posn },		// 11
	{ "right.posn",			"nnnnn",	&cond_right_posn },			// 12
	{ "in.motion.using.mouse", "",		&cond_unknown_13 }			// 13
};

AgiInstruction insV2[] = {
	{ "return",				"",			NULL },
	{ "increment",			"v",		&cmd_increment },
	{ "decrement",			"v",		&cmd_decrement },
	{ "assignn",			"vn",		&cmd_assignn },
	{ "assignv",			"vv",		&cmd_assignv },
	{ "addn",				"vn",		&cmd_addn },
	{ "addv",				"vv",		&cmd_addv },
	{ "subn",				"vn",		&cmd_subn },
	{ "subv",				"vv",		&cmd_subv },
	{ "lindirectv",			"vv",		&cmd_lindirectv },
	{ "lindirect",			"vv",		&cmd_rindirect },
	{ "lindirectn",			"vn",		&cmd_lindirectn },
	{ "set",				"n",		&cmd_set },
	{ "reset",				"n",		&cmd_reset },
	{ "toggle",				"n",		&cmd_toggle },
	{ "set.v",				"v",		&cmd_set_v },
	{ "reset.v",			"v",		&cmd_reset_v },
	{ "toggle.v",			"v",		&cmd_toggle_v },
	{ "new.room",			"n",		&cmd_new_room },
	{ "new.room.v",			"v",		&cmd_new_room_f },
	{ "load.logics",		"n",		&cmd_load_logic },
	{ "load.logics.v",		"v",		&cmd_load_logic_f },
	{ "call",				"n",		&cmd_call },
	{ "call.v",				"v",		&cmd_call_f },
	{ "load.pic",			"v",		&cmd_load_pic },
	{ "draw.pic",			"v",		&cmd_draw_pic },
	{ "show.pic",			"",			&cmd_show_pic },
	{ "discard.pic",		"v",		&cmd_discard_pic },
	{ "overlay.pic",		"v",		&cmd_overlay_pic },
	{ "show.pri.screen",	"",			&cmd_show_pri_screen },
	{ "load.view",			"n",		&cmd_load_view },
	{ "load.view.v",		"v",		&cmd_load_view_f },
	{ "discard.view",		"n",		&cmd_discard_view },
	{ "animate.obj",		"n",		&cmd_animate_obj },
	{ "unanimate.all",		"",			&cmd_unanimate_all },
	{ "draw",				"n",		&cmd_draw },
	{ "erase",				"n",		&cmd_erase },
	{ "position",			"nnn",		&cmd_position },
	{ "position.v",			"nvv",		&cmd_position_f },
	{ "get.posn",			"nvv",		&cmd_get_posn },
	{ "reposition",			"nvv",		&cmd_reposition },
	{ "set.view",			"nn",		&cmd_set_view },
	{ "set.view.v",			"nv",		&cmd_set_view_f },
	{ "set.loop",			"nn",		&cmd_set_loop },
	{ "set.loop.v",			"nv",		&cmd_set_loop_f },
	{ "fix.loop",			"n",		&cmd_fix_loop },
	{ "release.loop",		"n",		&cmd_release_loop },
	{ "set.cel",			"nn",		&cmd_set_cel },
	{ "set.cel.v",			"nv",		&cmd_set_cel_f },
	{ "last.cel",			"nv",		&cmd_last_cel },
	{ "current.cel",		"nv",		&cmd_current_cel },
	{ "current.loop",		"nv",		&cmd_current_loop },
	{ "current.view",		"nv",		&cmd_current_view },
	{ "number.of.loops",	"nv",		&cmd_number_of_loops },
	{ "set.priority",		"nn",		&cmd_set_priority },
	{ "set.priority.v",		"nv",		&cmd_set_priority_f },
	{ "release.priority",	"n",		&cmd_release_priority },
	{ "get.priority",		"nn",		&cmd_get_priority },
	{ "stop.update",		"n",		&cmd_stop_update },
	{ "start.update",		"n",		&cmd_start_update },
	{ "force.update",		"n",		&cmd_force_update },
	{ "ignore.horizon",		"n",		&cmd_ignore_horizon },
	{ "observe.horizon",	"n",		&cmd_observe_horizon },
	{ "set.horizon",		"n",		&cmd_set_horizon },
	{ "object.on.water",	"n",		&cmd_object_on_water },
	{ "object.on.land",		"n",		&cmd_object_on_land },
	{ "object.on.anything",	"n",		&cmd_object_on_anything },
	{ "ignore.objs",		"n",		&cmd_ignore_objs },
	{ "observe.objs",		"n",		&cmd_observe_objs },
	{ "distance",			"nnv",		&cmd_distance },
	{ "stop.cycling",		"n",		&cmd_stop_cycling },
	{ "start.cycling",		"n",		&cmd_start_cycling },
	{ "normal.cycle",		"n",		&cmd_normal_cycle },
	{ "end.of.loop",		"nn",		&cmd_end_of_loop },
	{ "reverse.cycle",		"n",		&cmd_reverse_cycle },
	{ "reverse.loop",		"nn",		&cmd_reverse_loop },
	{ "cycle.time",			"nv",		&cmd_cycle_time },
	{ "stop.motion",		"n",		&cmd_stop_motion },
	{ "start.motion",		"n",		&cmd_start_motion },
	{ "step.size",			"nv",		&cmd_step_size },
	{ "step.time",			"nv",		&cmd_step_time },
	{ "move.obj",			"nnnnn",	&cmd_move_obj },
	{ "move.obj.v",			"nvvvv",	&cmd_move_obj_f },
	{ "follow.ego",			"nnn",		&cmd_follow_ego },
	{ "wander",				"n",		&cmd_wander },
	{ "normal.motion",		"n",		&cmd_normal_motion },
	{ "set.dir",			"nv",		&cmd_set_dir },
	{ "get.dir",			"nv",		&cmd_get_dir },
	{ "ignore.blocks",		"n",		&cmd_ignore_blocks },
	{ "observe.blocks",		"n",		&cmd_observe_blocks },
	{ "block",				"nnnn",		&cmd_block },
	{ "unblock",			"",			&cmd_unblock },
	{ "get",				"n",		&cmd_get },
	{ "get.v",				"v",		&cmd_get_f },
	{ "drop",				"n",		&cmd_drop },
	{ "put",				"nn",		&cmd_put },
	{ "put.v",				"vv",		&cmd_put_f },
	{ "get.room.v",			"vv",		&cmd_get_room_f },
	{ "load.sound",			"n",		&cmd_load_sound },
	{ "sound",				"nn",		&cmd_sound },
	{ "stop.sound",			"",			&cmd_stop_sound },
	{ "print",				"s",		&cmd_print },
	{ "print.v",			"v",		&cmd_print_f },
	{ "display",			"nns",		&cmd_display },
	{ "display.v",			"vvv",		&cmd_display_f },
	{ "clear.lines",		"nns",		&cmd_clear_lines },
	{ "text.screen",		"",			&cmd_text_screen },
	{ "graphics",			"",			&cmd_graphics },
	{ "set.cursor.char",	"s",		&cmd_set_cursor_char },
	{ "set.text.attribute",	"nn",		&cmd_set_text_attribute },
	{ "shake.screen",		"n",		&cmd_shake_screen },
	{ "configure.screen",	"nnn",		&cmd_configure_screen },
	{ "status.line.on",		"",			&cmd_status_line_on },
	{ "status.line.off",	"",			&cmd_status_line_off },
	{ "set.string",			"ns",		&cmd_set_string },
	{ "get.string",			"ns",		&cmd_get_string },
	{ "word.to.string",		"nn",		&cmd_word_to_string },
	{ "parse",				"n",		&cmd_parse },
	{ "get.num",			"nv",		&cmd_get_num },
	{ "prevent.input",		"",			&cmd_prevent_input },
	{ "accept.input",		"",			&cmd_accept_input },
	{ "set.key",			"nnn",		&cmd_set_key },
	{ "add.to.pic",			"nnnnnnn",	&cmd_add_to_pic },
	{ "add.to.pic.v",		"vvvvvvv",	&cmd_add_to_pic_f },
	{ "status",				"",			&cmd_status },
	{ "save.game",			"",			&cmd_save_game },
	{ "restore.game",		"",			&cmd_load_game },
	{ "init.disk",			"",			&cmd_init_disk },
	{ "restart.game",		"",			&cmd_restart_game },
	{ "show.obj",			"n",		&cmd_show_obj },
	{ "random",				"nnv",		&cmd_random },
	{ "program.control",	"",			&cmd_program_control },
	{ "player.control",		"",			&cmd_player_control },
	{ "obj.status.v",		"v",		&cmd_obj_status_f },
	{ "quit",				"n",		&cmd_quit },
	{ "show.mem",			"",			&cmd_show_mem },
	{ "pause",				"",			&cmd_pause },
	{ "echo.line",			"",			&cmd_echo_line },
	{ "cancel.line",		"",			&cmd_cancel_line },
	{ "init.joy",			"",			&cmd_init_joy },
	{ "toggle.monitor",		"",			&cmd_toggle_monitor },
	{ "version",			"",			&cmd_version },
	{ "script.size",		"n",		&cmd_script_size },
	{ "set.game.id",		"s",		&cmd_set_game_id },
	{ "log",				"s",		&cmd_log },
	{ "set.scan.start",		"",			&cmd_set_scan_start },
	{ "reset.scan.start",	"",			&cmd_reset_scan_start },
	{ "reposition.to",		"nnn",		&cmd_reposition_to },
	{ "reposition.to.v",	"nvv",		&cmd_reposition_to_f },
	{ "trace.on",			"",			&cmd_trace_on },
	{ "trace.info", 		"nnn",		&cmd_trace_info },
	{ "print.at",			"snnn",		&cmd_print_at },
	{ "print.at.v",			"vnnn",		&cmd_print_at_v },
	{ "discard.view.v",		"v",		&cmd_discard_view},
	{ "clear.text.rect",	"nnnnn",	&cmd_clear_text_rect },
	{ "set.upper.left",		"nn",		&cmd_set_upper_left },
	{ "set.menu",			"s",		&cmd_set_menu },
	{ "set.menu.member",	"sn",		&cmd_set_menu_item },
	{ "submit.menu",		"",			&cmd_submit_menu },
	{ "enable.member",		"n",		&cmd_enable_item },
	{ "disable.member",		"n",		&cmd_disable_item },
	{ "menu.input",			"",			&cmd_menu_input },
	{ "show.obj.v",			"v",		&cmd_show_obj_v },
	{ "open.dialogue",		"",			&cmd_open_dialogue },
	{ "close.dialogue",		"",			&cmd_close_dialogue },
	{ "mul.n",				"vn",		&cmd_mul_n },
	{ "mul.v",				"vv",		&cmd_mul_v },
	{ "div.n",				"vn",		&cmd_div_n },
	{ "div.v",				"vv",		&cmd_div_v },
	{ "close.window",		"",			&cmd_close_window },
	{ "set.simple",			"n",		&cmd_set_simple },
	{ "push.script",		"",			&cmd_push_script },
	{ "pop.script",			"",			&cmd_pop_script },
	{ "hold.key",			"",			&cmd_hold_key },
	{ "set.pri.base",		"n",		&cmd_set_pri_base },
	{ "discard.sound",		"n",		&cmd_discard_sound },
	{ "hide.mouse",			"",			&cmd_hide_mouse },
	{ "allow.menu",			"n",		&cmd_allow_menu },
	{ "show.mouse",			"",			&cmd_show_mouse },
	{ "fence.mouse",		"nnnn",		&cmd_fence_mouse },
	{ "mouse.posn",			"vv",		&cmd_mouse_posn },
	{ "release.key",		"",			&cmd_release_key },
	{ "adj.ego.move.to.xy",	"",			&cmd_adj_ego_move_to_x_y }
};

void AgiEngine::setupOpcodes() {
	if (getVersion() >= 0x2000) {
		for (int i = 0; i <= ARRAYSIZE(insV2Test); ++i)
			_agiCondCommands[i] = insV2Test[i].func;
		for (int i = 0; i < ARRAYSIZE(insV2); ++i)
			_agiCommands[i] = insV2[i].func;

		logicNamesTest = insV2Test;
		logicNamesCmd = insV2;
	} else {
		for (int i = 0; i <= ARRAYSIZE(insV1Test); ++i)
			_agiCondCommands[i] = insV1Test[i].func;
		for (int i = 0; i < ARRAYSIZE(insV1); ++i)
			_agiCommands[i] = insV1[i].func;

		logicNamesTest = insV1Test;
		logicNamesCmd = insV1;
	}
}

}