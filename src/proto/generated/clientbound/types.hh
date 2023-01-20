// Automatically generated by generator/packet_types.py
#pragma once

#include <cstdint>

namespace mccpp::proto::generated {

namespace clientbound {
 namespace handshaking {
 }
 namespace play {
  struct add_entity_packet { static constexpr int32_t id = 0; };
  struct add_experience_orb_packet { static constexpr int32_t id = 1; };
  struct add_player_packet { static constexpr int32_t id = 2; };
  struct animate_packet { static constexpr int32_t id = 3; };
  struct award_stats_packet { static constexpr int32_t id = 4; };
  struct block_changed_ack_packet { static constexpr int32_t id = 5; };
  struct block_destruction_packet { static constexpr int32_t id = 6; };
  struct block_entity_data_packet { static constexpr int32_t id = 7; };
  struct block_event_packet { static constexpr int32_t id = 8; };
  struct block_update_packet { static constexpr int32_t id = 9; };
  struct boss_event_packet { static constexpr int32_t id = 10; };
  struct change_difficulty_packet { static constexpr int32_t id = 11; };
  struct clear_titles_packet { static constexpr int32_t id = 12; };
  struct command_suggestions_packet { static constexpr int32_t id = 13; };
  struct commands_packet { static constexpr int32_t id = 14; };
  struct container_close_packet { static constexpr int32_t id = 15; };
  struct container_set_content_packet { static constexpr int32_t id = 16; };
  struct container_set_data_packet { static constexpr int32_t id = 17; };
  struct container_set_slot_packet { static constexpr int32_t id = 18; };
  struct cooldown_packet { static constexpr int32_t id = 19; };
  struct custom_chat_completions_packet { static constexpr int32_t id = 20; };
  struct custom_payload_packet { static constexpr int32_t id = 21; };
  struct delete_chat_packet { static constexpr int32_t id = 22; };
  struct disconnect_packet { static constexpr int32_t id = 23; };
  struct disguised_chat_packet { static constexpr int32_t id = 24; };
  struct entity_event_packet { static constexpr int32_t id = 25; };
  struct explode_packet { static constexpr int32_t id = 26; };
  struct forget_level_chunk_packet { static constexpr int32_t id = 27; };
  struct game_event_packet { static constexpr int32_t id = 28; };
  struct horse_screen_open_packet { static constexpr int32_t id = 29; };
  struct initialize_border_packet { static constexpr int32_t id = 30; };
  struct keep_alive_packet { static constexpr int32_t id = 31; };
  struct level_chunk_with_light_packet { static constexpr int32_t id = 32; };
  struct level_event_packet { static constexpr int32_t id = 33; };
  struct level_particles_packet { static constexpr int32_t id = 34; };
  struct light_update_packet { static constexpr int32_t id = 35; };
  struct login_packet { static constexpr int32_t id = 36; };
  struct map_item_data_packet { static constexpr int32_t id = 37; };
  struct merchant_offers_packet { static constexpr int32_t id = 38; };
  struct pos { static constexpr int32_t id = 39; };
  struct pos_rot { static constexpr int32_t id = 40; };
  struct rot { static constexpr int32_t id = 41; };
  struct move_vehicle_packet { static constexpr int32_t id = 42; };
  struct open_book_packet { static constexpr int32_t id = 43; };
  struct open_screen_packet { static constexpr int32_t id = 44; };
  struct open_sign_editor_packet { static constexpr int32_t id = 45; };
  struct ping_packet { static constexpr int32_t id = 46; };
  struct place_ghost_recipe_packet { static constexpr int32_t id = 47; };
  struct player_abilities_packet { static constexpr int32_t id = 48; };
  struct player_chat_packet { static constexpr int32_t id = 49; };
  struct player_combat_end_packet { static constexpr int32_t id = 50; };
  struct player_combat_enter_packet { static constexpr int32_t id = 51; };
  struct player_combat_kill_packet { static constexpr int32_t id = 52; };
  struct player_info_remove_packet { static constexpr int32_t id = 53; };
  struct player_info_update_packet { static constexpr int32_t id = 54; };
  struct player_look_at_packet { static constexpr int32_t id = 55; };
  struct player_position_packet { static constexpr int32_t id = 56; };
  struct recipe_packet { static constexpr int32_t id = 57; };
  struct remove_entities_packet { static constexpr int32_t id = 58; };
  struct remove_mob_effect_packet { static constexpr int32_t id = 59; };
  struct resource_pack_packet { static constexpr int32_t id = 60; };
  struct respawn_packet { static constexpr int32_t id = 61; };
  struct rotate_head_packet { static constexpr int32_t id = 62; };
  struct section_blocks_update_packet { static constexpr int32_t id = 63; };
  struct select_advancements_tab_packet { static constexpr int32_t id = 64; };
  struct server_data_packet { static constexpr int32_t id = 65; };
  struct set_action_bar_text_packet { static constexpr int32_t id = 66; };
  struct set_border_center_packet { static constexpr int32_t id = 67; };
  struct set_border_lerp_size_packet { static constexpr int32_t id = 68; };
  struct set_border_size_packet { static constexpr int32_t id = 69; };
  struct set_border_warning_delay_packet { static constexpr int32_t id = 70; };
  struct set_border_warning_distance_packet { static constexpr int32_t id = 71; };
  struct set_camera_packet { static constexpr int32_t id = 72; };
  struct set_carried_item_packet { static constexpr int32_t id = 73; };
  struct set_chunk_cache_center_packet { static constexpr int32_t id = 74; };
  struct set_chunk_cache_radius_packet { static constexpr int32_t id = 75; };
  struct set_default_spawn_position_packet { static constexpr int32_t id = 76; };
  struct set_display_objective_packet { static constexpr int32_t id = 77; };
  struct set_entity_data_packet { static constexpr int32_t id = 78; };
  struct set_entity_link_packet { static constexpr int32_t id = 79; };
  struct set_entity_motion_packet { static constexpr int32_t id = 80; };
  struct set_equipment_packet { static constexpr int32_t id = 81; };
  struct set_experience_packet { static constexpr int32_t id = 82; };
  struct set_health_packet { static constexpr int32_t id = 83; };
  struct set_objective_packet { static constexpr int32_t id = 84; };
  struct set_passengers_packet { static constexpr int32_t id = 85; };
  struct set_player_team_packet { static constexpr int32_t id = 86; };
  struct set_score_packet { static constexpr int32_t id = 87; };
  struct set_simulation_distance_packet { static constexpr int32_t id = 88; };
  struct set_subtitle_text_packet { static constexpr int32_t id = 89; };
  struct set_time_packet { static constexpr int32_t id = 90; };
  struct set_title_text_packet { static constexpr int32_t id = 91; };
  struct set_titles_animation_packet { static constexpr int32_t id = 92; };
  struct sound_entity_packet { static constexpr int32_t id = 93; };
  struct sound_packet { static constexpr int32_t id = 94; };
  struct stop_sound_packet { static constexpr int32_t id = 95; };
  struct system_chat_packet { static constexpr int32_t id = 96; };
  struct tab_list_packet { static constexpr int32_t id = 97; };
  struct tag_query_packet { static constexpr int32_t id = 98; };
  struct take_item_entity_packet { static constexpr int32_t id = 99; };
  struct teleport_entity_packet { static constexpr int32_t id = 100; };
  struct update_advancements_packet { static constexpr int32_t id = 101; };
  struct update_attributes_packet { static constexpr int32_t id = 102; };
  struct update_enabled_features_packet { static constexpr int32_t id = 103; };
  struct update_mob_effect_packet { static constexpr int32_t id = 104; };
  struct update_recipes_packet { static constexpr int32_t id = 105; };
  struct update_tags_packet { static constexpr int32_t id = 106; };
 }
 namespace status {
  struct status_response_packet { static constexpr int32_t id = 0; };
  struct pong_response_packet { static constexpr int32_t id = 1; };
 }
 namespace login {
  struct login_disconnect_packet { static constexpr int32_t id = 0; };
  struct hello_packet { static constexpr int32_t id = 1; };
  struct game_profile_packet { static constexpr int32_t id = 2; };
  struct login_compression_packet { static constexpr int32_t id = 3; };
  struct custom_query_packet { static constexpr int32_t id = 4; };
 }
}

}
