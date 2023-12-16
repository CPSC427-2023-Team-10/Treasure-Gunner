#include "ui_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "world_init.hpp"

#include <imgui_internal.h>
#include <fstream>
#include <iostream>
#include <string>

UISystem::UISystem() {
	state = UI_STATE_ID::MENU;
}

bool UISystem::init(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();
	// load Fonts
	load_fonts();
	ImGuiStyle* style = &ImGui::GetStyle();
	style->Colors[ImGuiCol_Text] = ui_font_color_u32;
	this->window = window;
	return true;
}

// Font from https://www.1001fonts.com/vinque-font.html
void UISystem::load_fonts() {
	ImGuiIO& io = ImGui::GetIO();
	std::string fontPath = font_path("HIVNotRetro-Regular.otf");
	fonts[(int)FONT_SIZE_ID::EXTRA_SMALL] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 18.0f);
	fonts[(int)FONT_SIZE_ID::SMALL] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 24.0f);
	fonts[(int)FONT_SIZE_ID::MEDIUM] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 32.0f);
	fonts[(int)FONT_SIZE_ID::LARGE] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 60.0f);
	fonts[(int)FONT_SIZE_ID::EXTRA_LARGE] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 92.0f);
	fonts[(int)FONT_SIZE_ID::TITLE] = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), 120.0f);

}

void UISystem::set_textures(std::array<GLuint, texture_count>& textures) {
	this->textures = textures;
	init_gun_textures();
}

void UISystem::init_gun_textures() {
	gun_textures[(int)GUN_ID::STRAIGHT_SHOT] = textures[(GLuint)TEXTURE_ASSET_ID::STRAIGHT_SHOT];
	gun_textures[(int)GUN_ID::SPLIT_SHOT] = textures[(GLuint)TEXTURE_ASSET_ID::SPLIT_SHOT];
	gun_textures[(int)GUN_ID::BOUNCING_SHOT] = textures[(GLuint)TEXTURE_ASSET_ID::BOUNCE_SHOT];
	gun_textures[(int)GUN_ID::LONG_SHOT] = textures[(GLuint)TEXTURE_ASSET_ID::LONG_SHOT];
	gun_textures[(int)GUN_ID::RAPID_SHOT] = textures[(GLuint)TEXTURE_ASSET_ID::RAPID_SHOT];
}

// Enters the given state
// Some state combinations will produce undefined behavior
void UISystem::enter_state(UI_STATE_ID state) {
	this->state = (UI_STATE_ID)((int)this->state | (int)state);
}

// Exits the given state
// Be sure that the UI will have a state even after this is called
void UISystem::exit_state(UI_STATE_ID state) {
	this->state = (UI_STATE_ID)((int)this->state & ~(int)state);
}

void UISystem::toggle_state(UI_STATE_ID state) {
	this->state = (UI_STATE_ID)((int)this->state ^ (int)state);
}

bool UISystem::in_state(UI_STATE_ID state) {
	return (int)this->state & (int)state;
}

bool UISystem::in_pause_state() {
	return in_state(UI_STATE_ID::PAUSE) || in_state(UI_STATE_ID::CONTROLS) || in_state(UI_STATE_ID::MENU) || in_state(UI_STATE_ID::SCORE);
}

bool UISystem::wants_mouse_input() {
	ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

void UISystem::render() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	pressed_button = UI_BUTTON_ID::NONE;

	if (resolutionScale != 1.f) {
		// Render to an internal size of 1920x1080
		if (glfwGetWindowMonitor(window)) {
			// Fullscreen
			ImGui::GetMainViewport()->Size = ImVec2(window_width_px, window_height_px);
			ImGui::GetMainViewport()->WorkSize = ImVec2(window_width_px, window_height_px);
		}
		else {
			// Windowed
			ImGui::GetMainViewport()->Size = ImVec2(window_width_px, window_height_px - toolbar_height);
			ImGui::GetMainViewport()->WorkSize = ImVec2(window_width_px, window_height_px - toolbar_height);
		}
		
	}
	// Show messages
	for (auto& message : registry.messages.components) {
		show_message(message);
	}

	for (auto& dialogue : registry.dialogues.components) {
		show_dialogue(dialogue);
	}

	for (auto& boss : registry.bosses.components) {
		boss_health_bar(boss);
	}

	if (registry.screenStates.size() > 0) {
		ScreenState& screenState = registry.screenStates.components[0];
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(window_width_px, window_height_px));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, screenState.screen_darken_factor));
		ImGui::Begin("Screen State", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::End();
		ImGui::PopStyleColor();
	}

	if (in_state(UI_STATE_ID::MENU)) {
		show_menu();
	}

	if (in_state(UI_STATE_ID::SCORE)) {
		show_score();
	}

	if (in_state(UI_STATE_ID::PLAYING)) {
		show_playing();
	}

	if (in_state(UI_STATE_ID::DEATH)) {
		show_death();
	}

	if (in_state(UI_STATE_ID::PAUSE)) {
		show_pause();
	}

	if (in_state(UI_STATE_ID::CONTROLS)) {
		show_controls();
	}

	for (int i = 0; i < registry.alerts.size(); i++) {
		Alert alert = registry.alerts.components[i];
		if (show_alert(alert)) {
			registry.remove_all_components_of(registry.alerts.entities[i]);
		}
	}
	ImGui::Render();
	if (resolutionScale != 1.0f) {
		// Render to the actual window size
		ImGui::GetDrawData()->DisplaySize = ImVec2(resolutionWidth, resolutionHeight);
	}
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

UISystem::~UISystem() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void UISystem::show_menu() {
	// Show Main menu
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImVec2 window_size = ImVec2(768, 780);
	ImVec2 button_size = ImVec2(340, 120);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("Main Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(ImVec2(window_width_px, window_height_px));
	ImGui::SetCursorPos(ImVec2(0, 0));
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::TITLE_SCREEN], ImVec2(window_width_px, window_height_px));
	ImGui::SetCursorPos(ImVec2(window_width_px - button_size.x - 25, 25));
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - window_size.x / 2, window_height_px / 2 - window_size.y / 2));
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::TITLE_SCREEN_PANEL], window_size);
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::TITLE]);
	ImVec2 text_size = ImGui::CalcTextSize("Treasure");
	// render shadow
	ImGui::PushStyleColor(ImGuiCol_Text, font_shadow_color_u32);
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - text_size.x / 2 + 3, window_height_px / 2 - window_size.y / 2 + 50 + 3));
	ImGui::Text("Treasure");
	ImGui::PopStyleColor();
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - text_size.x/2, window_height_px/2 - window_size.y / 2 + 50));
	ImGui::Text("Treasure");
	text_size = ImGui::CalcTextSize("Gunner");
	// render shadow
	ImGui::PushStyleColor(ImGuiCol_Text, font_shadow_color_u32);
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - text_size.x / 2 + 3, window_height_px / 2 - window_size.y / 2 + 50 + text_size.y + 3));
	ImGui::Text("Gunner");
	ImGui::PopStyleColor();
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - text_size.x / 2, window_height_px / 2 - window_size.y / 2 + 50 + text_size.y));
	ImGui::Text("Gunner");
	ImGui::PopFont();
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::LARGE]);
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - button_size.x / 2, window_height_px / 2 + window_size.y / 2 - 3 * button_size.y - 50));
	if (menu_button("New Game", button_size)) {
		pressed_button = UI_BUTTON_ID::NEW_GAME;
	}
	ImGui::SetCursorPosX(window_width_px / 2 - button_size.x / 2);
	if (menu_button("Load Game", button_size)) {
		// Load Game
		pressed_button = UI_BUTTON_ID::LOAD_GAME;
	}
	ImGui::SetCursorPosX(window_width_px / 2 - button_size.x / 2);
	if (menu_button("Quit", button_size)) {
		pressed_button = UI_BUTTON_ID::QUIT;
		glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
	}
	ImGui::PopFont();
	ImGui::End();
	ImGui::PopStyleColor();
	
}

void UISystem::show_pause() {
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::LARGE]);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15);
	ImVec2 button_size = ImVec2(340, 120);
	ImVec2 window_size = ImVec2(431, 696);
	int padding = ImGui::GetStyle().WindowPadding.x;
	ImGui::SetNextWindowPos(ImVec2(window_width_px / 2 - window_size.x / 2 - padding, window_height_px / 2 - window_size.y / 2));
	ImGui::Begin("Pause", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(window_size);
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::MENU_PANEL], window_size);
	ImGui::SetCursorPos(ImVec2(window_size.x / 2 - button_size.x / 2 + padding, window_size.y / 2 - 2.5 * button_size.y + 8));
	if (menu_button("Resume", button_size)) {
		pressed_button = UI_BUTTON_ID::RESUME;
		exit_state(UI_STATE_ID::PAUSE);
	}
	ImGui::SetCursorPosX(window_size.x / 2 - button_size.x / 2 + padding);
	if (menu_button("Save Game", button_size)) {
		pressed_button = UI_BUTTON_ID::SAVE_GAME;
	}
	ImGui::SetCursorPosX(window_size.x / 2 - button_size.x / 2 + padding);
	if (menu_button("Main Menu", button_size)) {
		pressed_button = UI_BUTTON_ID::MAIN_MENU;
		exit_state(UI_STATE_ID::PAUSE);
		exit_state(UI_STATE_ID::PLAYING);
		enter_state(UI_STATE_ID::MENU);
	}
	ImGui::SetCursorPosX(window_size.x / 2 - button_size.x / 2 + padding);
	if (menu_button("Controls", button_size)) {
		pressed_button = UI_BUTTON_ID::CONTROLS;
		exit_state(UI_STATE_ID::PAUSE);
		exit_state(UI_STATE_ID::PLAYING);
		enter_state(UI_STATE_ID::CONTROLS);
	}
	ImGui::SetCursorPosX(window_size.x / 2 - button_size.x / 2 + padding);
	if (menu_button("Quit", button_size)) {
		pressed_button = UI_BUTTON_ID::QUIT;
		glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
	}
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::PopFont();
}

void UISystem::show_death() {
	// Show death screen
}

void UISystem::show_score() {
	Player& player = registry.players.components[0];
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 1.f));
	ImVec2 window_size = ImVec2(768, 780);
	ImVec2 button_size = ImVec2(340, 120);
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("Result Menu", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(ImVec2(window_width_px, window_height_px));
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - window_size.x / 2, window_height_px / 2 - window_size.y / 2));
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::TITLE_SCREEN_PANEL], window_size);
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::TITLE]);
	int coins = registry.players.components[0].coins;
	std::string coin_count = " X " + std::to_string(coins);
	int digits = std::to_string(coins).length();
	ImVec2 coin_count_size = ImGui::CalcTextSize(coin_count.c_str());
	ImVec2 coin_size = ImVec2(coin_count_size.y, coin_count_size.y);
	ImVec2 text_size = ImGui::CalcTextSize("- RESULT -");

	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - text_size.x / 2 + 3, window_height_px / 2 - window_size.y / 2 + 80 + 3));
	ImGui::Text("- RESULT -");
	ImGui::SetCursorPosX(window_width_px / 2 - button_size.x / 2 - 30 * (digits - 1));
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::COIN], coin_size );
	ImGui::SameLine();
	ImGui::Text(coin_count.c_str());
	ImGui::PopFont();

	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::LARGE]);
	ImGui::SetCursorPos(ImVec2(window_width_px / 2 - button_size.x / 2, window_height_px / 2 + window_size.y / 2 - 3 * button_size.y - 50));
	if (menu_button("New Game", button_size)) {
		pressed_button = UI_BUTTON_ID::NEW_GAME;
	}
	ImGui::SetCursorPosX(window_width_px / 2 - button_size.x / 2);
	if (menu_button("Load Game", button_size)) {
		// Load Game
		pressed_button = UI_BUTTON_ID::LOAD_GAME;
	}
	ImGui::SetCursorPosX(window_width_px / 2 - button_size.x / 2);
	if (menu_button("Quit", button_size)) {
		pressed_button = UI_BUTTON_ID::QUIT;
		glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
	}
	ImGui::PopFont();
	ImGui::End();
	ImGui::PopStyleColor();


}

void UISystem::show_playing() {
	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
	draw_list->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::UI_PANEL], ImVec2(window_width_px-720, -70), ImVec2(window_width_px, 110), ImVec2(0, 0), ImVec2(1, 1));
	health_bar();
	coin_counter();
	range_counter();
	hotbar();
}

void UISystem::show_controls() {
	// Add heart background
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	ImVec2 button_size = ImVec2(100, 100);
	ImGui::SetNextWindowPos(ImVec2(0 - padding.x, 0 - padding.y));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin("Controls", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::TUTORIAL], ImVec2(window_width_px, window_height_px));
	ImGui::SetCursorPos(ImVec2(window_width_px - button_size.x - 25, 25));
	if (ImGui::ImageButton("##", (void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::EXIT_BUTTON], button_size)) {
		exit_state(UI_STATE_ID::CONTROLS);
		enter_state(UI_STATE_ID::PAUSE);
		enter_state(UI_STATE_ID::PLAYING);
	}
	ImGui::End();
	ImGui::PopStyleColor(3);
}

void UISystem::show_message(Message& message) {
	ImGui::PushFont(fonts[(int)message.font_size]);
	ImVec2 text_size = ImGui::CalcTextSize(message.message.c_str());
	ImGui::SetNextWindowPos(ImVec2(message.position.x - text_size.x/2, message.position.y - text_size.y/2));
	ImGui::Begin(message.message.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(ImVec2(0, 0));
	ImGui::Text(message.message.c_str());
	ImGui::End();
	ImGui::PopFont();
}

void UISystem::show_dialogue(Dialogue& dialogue) {
	int dialogue_width = 882;
	ImGui::Begin(dialogue.speaker.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImVec2 window_padding = window->WindowPadding;
	ImGui::SetWindowPos(ImVec2(window_width_px/2 - window_padding.x - dialogue_width/2, window_height_px - 300));
	ImGui::SetWindowSize(ImVec2(0, 0));
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::MEDIUM]);
	if (dialogue_box(dialogue.dialogue_pages[dialogue.current_page], ImVec2(882, 180), dialogue.current_character)) {
		if (!in_pause_state()) {
			if (dialogue.current_character < dialogue.dialogue_pages[dialogue.current_page].size()) {
				dialogue.current_character = dialogue.dialogue_pages[dialogue.current_page].size();
			}
			else {
				dialogue.current_page++;
				dialogue.current_character = 0;
			}
		}
	}
	ImGui::PopFont();
	ImGui::End();
}

bool UISystem::show_alert(Alert& alert) {
	ImVec2 window_size = ImVec2(window_width_px, window_height_px);
	ImVec2 panel_size = ImVec2(498, 180);
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::MEDIUM]);
	ImVec2 text_size = ImGui::CalcTextSize(alert.message.c_str());
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin(alert.message.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(window_size);
	ImGui::SetCursorPos(ImVec2(window_size.x / 2 - panel_size.x / 2, window_size.y / 2 - panel_size.y / 2));
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::ALERT_PANEL], panel_size, ImVec2(0, 0), ImVec2(1, 1));
	ImGui::SetCursorPos(ImVec2(window_size.x / 2 - text_size.x / 2, window_size.y/2 - panel_size.y/2 + 50));
	ImGui::Text(alert.message.c_str());
	ImGui::SetCursorPos(ImVec2(window_size.x / 2 - 170 / 2, window_size.y / 2 - panel_size.y / 2 + 100));
	bool pressed = menu_button("OK", ImVec2(170, 60));
	ImGui::End();
	ImGui::PopFont();
	return pressed;
}

void UISystem::health_bar() {
	Player& player = registry.players.components[0];
	float health_bar_width = 58 * player.max_health/20;
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::MEDIUM]);
	std::string health_bar = "- Health: " + std::to_string((int)player.health) + "/" + std::to_string((int)player.max_health) + " -";
	float text_height = ImGui::CalcTextSize(health_bar.c_str()).y;
	ImGui::SetNextWindowPos(ImVec2(window_width_px -health_bar_width - 50, 0));
	ImGui::Begin("Health", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::SetWindowSize(ImVec2(health_bar_width, text_height));
	center_text(health_bar.c_str());
	ImGui::End();
	ImGui::PopFont();

	float health_idx = player.max_health - 20;
	float curr_health = player.health;
	for (int i = 0; i * 20 < player.max_health; i++) {
		Transform transform;
		vec2 position = vec2(window_width_px - i * 58 - 29 - 50, 26 + text_height + 7);
		float health_bucket = std::max(0.f, curr_health - health_idx);
		float filled = health_bucket / 20.f;
		health_idx -= 20;
		curr_health -= health_bucket;
		heart(filled, position);
	}
}

void UISystem::coin_counter() {
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::MEDIUM]);
	std::string coin_count = " X " + std::to_string(registry.players.components[0].coins);
	ImVec2 coin_count_size = ImGui::CalcTextSize(coin_count.c_str());
	ImVec2 coin_size = ImVec2(coin_count_size.y, coin_count_size.y);
	ImGui::SetNextWindowPos(ImVec2(window_width_px - 500, 0));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin("Coins", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	center_text("- Coins -");
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::COIN], coin_size);
	ImGui::SameLine();
	ImGui::Text(coin_count.c_str());
	ImGui::End();
	ImGui::PopFont();
}

void UISystem::range_counter() {
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::MEDIUM]);
	std::string range_count = std::to_string((int)registry.players.components[0].range);
	ImVec2 range_size = ImVec2(48, 48);
	ImGui::SetNextWindowPos(ImVec2(window_width_px - 680, 0));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin("Range", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	center_text("- Range -");
	int y_offset = 5;
	ImVec2 cursor_pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(cursor_pos.x, cursor_pos.y - y_offset));
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::RANGE_ICON], range_size);
	ImGui::SameLine();
	cursor_pos = ImGui::GetCursorPos();
	ImGui::SetCursorPos(ImVec2(cursor_pos.x, cursor_pos.y + y_offset));
	ImGui::Text(range_count.c_str());
	ImGui::End();
	ImGui::PopFont();

}

void UISystem::hotbar() {
	// Show hotbar
	Entity player = registry.players.entities[0];
	Hotbar& hotbar = registry.hotbars.get(player);
	ImGui::SetNextWindowPos(ImVec2(25, -8));
	ImGui::SetNextWindowSize(ImVec2(0, 0));
	ImGui::Begin("Hotbar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	for (int i = 0; i < hotbar.capacity; i++) {
		if (hotbar_panel(hotbar.guns[i], i, hotbar.selected == i, ImVec2(81, 100))) {
			hotbar.selected = i;
			swapGun(player, i);
		}

		// Drag and Drop guns to different inventory slots
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
			ImGui::SetDragDropPayload("GUN", &i, sizeof(int));
			if (hotbar.guns[i] != GUN_ID::NO_GUN) {
				ImGui::Image((void*)(intptr_t)gun_textures[(int)hotbar.guns[i]], ImVec2(64, 64));
			}
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GUN")) {
				// Swap guns
				GUN_ID temp = hotbar.guns[i];
				hotbar.guns[i] = hotbar.guns[*(int*)payload->Data];
				hotbar.guns[*(int*)payload->Data] = temp;
				swapGun(player, hotbar.selected);
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
	}
	ImGui::End();
}

void UISystem::heart(float filled, vec2 pos) {
	ImVec2 position = ImVec2(pos.x, pos.y);
	ImVec2 size = ImVec2(58, 51);
	ImVec2 p_min = ImVec2(position.x - size.x / 2, position.y - size.y / 2);
	ImVec2 p_max = ImVec2(position.x + size.x / 2, position.y + size.y / 2);
	GLuint heart_texture = textures[(GLuint)TEXTURE_ASSET_ID::HEART];
	ImDrawList* draw_list = ImGui::GetForegroundDrawList();
	// Add heart background
	draw_list->AddImage((void*)(intptr_t)heart_texture, p_min, p_max, ImVec2(0, 0), ImVec2(1, 0.5));
	// Add heart foreground
	draw_list->AddImage((void*)(intptr_t)heart_texture, ImVec2(p_min.x, p_min.y + size.y * (1-filled)), p_max, ImVec2(0, 0.5 + (1 - filled)/2), ImVec2(1, 1));
}

void UISystem::boss_health_bar(Boss& boss) {
	ImGui::PushFont(fonts[(int)FONT_SIZE_ID::MEDIUM]);
	ImVec2 size = ImVec2(808, 192);
	ImVec2 padding = ImGui::GetStyle().WindowPadding;
	std::string text = std::to_string((int)boss.health) + "/" + std::to_string((int)boss.max_health);
	ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
	ImGui::SetNextWindowPos(ImVec2(window_width_px / 2 - size.x / 2 - padding.x, window_height_px - 215));
	ImGui::Begin("Boss Health", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::Image((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::BOSS_HEALTH_BAR], size);
	ImGui::SetCursorPos(ImVec2(size.x / 2 - text_size.x / 2, size.y / 2 - text_size.y / 2));

	ImVec2 rect_min = ImVec2(window_width_px / 2 - size.x / 2 + 16, window_height_px - 215 + 64 + padding.y);
	ImVec2 rect_max = ImVec2(window_width_px / 2 + size.x / 2 - 16, window_height_px - 215 + 152 + padding.y);
	ImVec2 rect_size = ImVec2(rect_max.x - rect_min.x, rect_max.y - rect_min.y);
	ImVec2 rect_filled_size = ImVec2(rect_size.x * boss.health / boss.max_health, rect_size.y);
	rect_max.x = rect_min.x + rect_filled_size.x;
	// Draw red rectangle
	ImGui::GetWindowDrawList()->AddRectFilled(rect_min, rect_max, ImGui::GetColorU32(ImVec4(0.5f, 0.f, 0.f, 1.f)));

	ImGui::SetCursorPos(ImVec2(size.x / 2 - text_size.x / 2, size.y / 2 - text_size.y / 2 + 15));
	ImGui::Text(text.c_str());
	ImGui::End();
	ImGui::PopFont();
}

void UISystem::name_plate(std::string name, ImVec2 size) {

}

bool UISystem::dialogue_box(std::string text, ImVec2 size, int characters) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) {
		return false;
	}
	ImGui::PushID(text.c_str());
	ImGuiID id = ImGui::GetID(text.c_str());
	ImGui::PopID();

	ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id)) {
		return false;
	}

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	window->DrawList->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::DIALOGUE_PANEL], bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

	const ImVec2 text_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
	const char* txt = text.c_str();
	// Render text
	ImGui::RenderTextClipped(ImVec2(bb.Min.x + 21, bb.Min.y + 15), ImVec2(bb.Max.x - 21, bb.Max.y - 15), txt, txt + characters, &text_size, ImVec2(0, 0), &bb);


	// Render arrow
	window->DrawList->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::DIALOGUE_ARROW], ImVec2(bb.Max.x - 55, bb.Max.y - 55), ImVec2(bb.Max.x - 28, bb.Max.y - 25), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
	return pressed;
}

bool UISystem::menu_button(std::string text, ImVec2 size) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) {
		return false;
	}
	ImGui::PushID(text.c_str());
	ImGuiID id = ImGui::GetID(text.c_str());
	ImGui::PopID();

	ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id)) {
		return false;
	}

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	const ImU32 col = ImGui::GetColorU32(hovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.f) : ImVec4(0.7f, 0.7f, 0.7f, 1.0f));


	if (held) {
		// Render the button pressed image
		window->DrawList->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::MENU_BUTTON_PRESSED], bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
	}
	else {
		// Render the button normal image
		window->DrawList->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::MENU_BUTTON], bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
	}

	
	const ImVec2 text_size = ImGui::CalcTextSize(text.c_str(), NULL, true);
	// Render text shadow
	ImGui::PushStyleColor(ImGuiCol_Text, font_shadow_color_u32);
	ImGui::RenderTextClipped(ImVec2(bb.Min.x + 3, bb.Min.y + 3), ImVec2(bb.Max.x + 3, bb.Max.y + 3), text.c_str(), NULL, &text_size, ImVec2(0.5f, 0.5f), &bb);
	ImGui::PopStyleColor();
	// Render text
	ImGui::RenderTextClipped(bb.Min, bb.Max, text.c_str(), NULL, &text_size, ImVec2(0.5f, 0.5f), &bb);
	
	return pressed;
}

bool UISystem::hotbar_panel(GUN_ID gun, int index, bool selected, ImVec2 size) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) {
		return false;
	}
	ImGui::PushID(index);
	ImGuiID id = ImGui::GetID(&index);
	ImGui::PopID();
	

	ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + size.x, window->DC.CursorPos.y + size.y));

	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, id)) {
		return false;
	}

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

	const ImU32 col = ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f));


	if (selected) {
		// Render the button pressed image
		window->DrawList->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::HOTBAR_PANEL_SELECTED], bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
	}
	else {
		// Render the button normal image
		window->DrawList->AddImage((void*)(intptr_t)textures[(GLuint)TEXTURE_ASSET_ID::HOTBAR_PANEL], bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
	}

	// Render gun
	if (gun != GUN_ID::NO_GUN) {
		// draw gun centered in box with size 64x64
		ImVec2 center = ImVec2(bb.Min.x + (bb.Max.x - bb.Min.x) / 2, bb.Min.y + (bb.Max.y - bb.Min.y) / 2);
		ImVec2 gun_size = ImVec2(64, 64);
		ImVec2 gun_min = ImVec2(center.x - gun_size.x / 2, center.y - gun_size.y / 2);
		ImVec2 gun_max = ImVec2(center.x + gun_size.x / 2, center.y + gun_size.y / 2);
		window->DrawList->AddImage((void*)(intptr_t)gun_textures[(int)gun], gun_min, gun_max, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)));
	}
	
	return pressed;
}

void UISystem::center_text(const char* text) {
	ImVec2 text_size = ImGui::CalcTextSize(text);
	ImGui::SetCursorPosX((ImGui::GetWindowSize().x - text_size.x) / 2);
	ImGui::Text(text);
}


void createTitleScreenArt(RenderSystem* renderer, vec2 pos, float width, float height)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(width, height);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TITLE_SCREEN,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		(unsigned int)RENDER_LAYER_ID::BACKGROUND_FAR);
}