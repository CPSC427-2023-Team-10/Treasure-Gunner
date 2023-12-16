#pragma once
#include "common.hpp"
#include "components.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class UISystem {

public:
	UISystem();
	bool init(GLFWwindow* window);
	bool in_state(UI_STATE_ID state);
	bool in_pause_state();
	bool wants_mouse_input();
	void set_textures(std::array<GLuint, texture_count>& textures);
	void enter_state(UI_STATE_ID state);
	void exit_state(UI_STATE_ID state);
	void toggle_state(UI_STATE_ID state);
	void render();

	UI_STATE_ID state = UI_STATE_ID::PLAYING;
	UI_BUTTON_ID pressed_button = UI_BUTTON_ID::NONE;
	~UISystem();
private:
	
	std::array<GLuint, texture_count> textures = std::array<GLuint, texture_count>();
	std::array<GLuint, gun_count> gun_textures = std::array<GLuint, gun_count>();
	std::array<ImFont*, font_size_count> fonts = std::array<ImFont*, font_size_count>();
	ImVec4 ui_font_color_u32 = ImVec4(1.f, 0.969, 0.761, 1.f);
	ImVec4 alt_font_color = ImVec4(1.f, 0.714, 0.337, 1.f);
	ImVec4 font_shadow_color_u32 = ImVec4(0.208f, 0.169f, 0.18f, 1.f);
	static const int toolbar_height = 25;
	GLFWwindow* window;
	void load_fonts();
	void init_gun_textures();

	void show_menu();
	void show_pause();
	void show_death();
	void show_score();
	void show_playing();
	void show_controls();
	void show_dialogue(Dialogue& dialogue);
	void show_message(Message& message);
	bool show_alert(Alert& alert);

	bool menu_button(std::string text, ImVec2 size);
	bool dialogue_box(std::string text, ImVec2 size, int characters);
	bool hotbar_panel(GUN_ID gun, int index, bool selected, ImVec2 size);
	void name_plate(std::string name, ImVec2 size);
	void boss_health_bar(Boss& boss);

	void coin_counter();
	void health_bar();
	void range_counter();
	void hotbar();
	void heart(float filled, vec2 pos);
	void center_text(const char* text);
};