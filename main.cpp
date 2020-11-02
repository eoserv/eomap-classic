
#include "common.hpp"
#include <allegro5/allegro_physfs.h>
#include <physfs.h>

#include "EO_Map.hpp"
#include "Map_Renderer.hpp"
#include "Palette.hpp"
#ifdef WIN32
#include "GUI.hpp"
#endif // WIN32
#include "eodata.hpp"

#include "resource.h"

#define Q_REGISTER_ALL() \
{ \
	q.Register(map_display); \
	q.Register(pal_display); \
	q.Register(keyboard); \
	q.Register(mouse); \
	q.Register(timer); \
	q.Register(anim_timer); \
	al_register_event_source(q, al_menu_source); \
}

#define Q_UNREGISTER_ALL() \
{ \
	q.Unregister(map_display); \
	q.Unregister(pal_display); \
	q.Unregister(keyboard); \
	q.Unregister(mouse); \
	q.Unregister(timer); \
	q.Unregister(anim_timer); \
	al_unregister_event_source(q, al_menu_source); \
}
/*
void ah(const char* expr, const char* file, int line, const char* func)
{
	printf("Assertion failed %s:%d (%s):\n%s", file, line, func, expr);
	fflush(stdout);
	std::abort();
}
*/
static void print_physfs_error()
{
	std::string str;

	PHYSFS_ErrorCode errcode = PHYSFS_getLastErrorCode();
	const char* errstr = PHYSFS_getErrorByCode(errcode);

	ALLEGRO_USTR *ustr = al_ustr_newf(
		"physfs error %u: %s\n", (unsigned)errcode, errstr
	);
	str = al_cstr(ustr);
	al_ustr_free(ustr);

	al_show_native_message_box(nullptr, "Error", "Initialization error",
		str.c_str(), nullptr, ALLEGRO_MESSAGEBOX_ERROR);
}

extern "C" char builtin_data[];
extern "C" std::size_t builtin_data_size;

std::string g_eo_install_path;

bool has_all_eo_files(std::string path)
{
	int required_gfx_files[] = {3, 4, 5, 6, 7, 22};

	for (int i : required_gfx_files)
	{
		char suffix[sizeof "/gfx/gfx.egf" + 3];
		snprintf(suffix, sizeof suffix, "/gfx/gfx%03i.egf", i);

		std::string filename = path + suffix;

		cio::stream test_file(filename.c_str(), "rb");

		if (!test_file)
			return false;
	}

	return true;
}

char hardcoded_install_paths[][64] = {
	".",
	"%ProgramFiles(x86)%\\EndlessOnline",
	"\\Program Files (x86)\\EndlessOnline",
	"C:\\Program Files (x86)\\EndlessOnline",
	"%ProgramFiles%\\EndlessOnline",
	"%ProgramFiles6432%\\EndlessOnline",
	"\\Program Files\\EndlessOnline",
	"C:\\Program Files\\EndlessOnline"
};

static bool try_get_eo_registry_path(const char* key, const char* key2)
{
	HKEY registry;

	char reg_path[1024];
	DWORD reg_path_type = REG_SZ;
	DWORD reg_path_size = sizeof reg_path;

	{
		int result = RegCreateKeyEx(
			HKEY_CURRENT_USER, key, 0, 0, 0,
			KEY_READ, 0, &registry, 0
		);

		if (result != ERROR_SUCCESS)
			return false;
	}

	{
		int result = RegQueryValueEx(
			registry, key2, 0, &reg_path_type,
			reinterpret_cast<BYTE *>(&reg_path), &reg_path_size
		);

		if (result != ERROR_SUCCESS)
		{
			RegCloseKey(registry);
			return false;
		}
	}

	RegCloseKey(registry);

	if (has_all_eo_files(reg_path))
	{
		g_eo_install_path = reg_path;
		return true;
	}

	return false;
}

static bool try_hardcoded_path(const char* path)
{
	char expanded_path[1024];

	if (ExpandEnvironmentStringsA(path, expanded_path, sizeof expanded_path))
	{
		if (has_all_eo_files(expanded_path))
		{
			g_eo_install_path = expanded_path;
			return true;
		}
	}
	else if (std::strchr(path, '%') == nullptr)
	{
		if (has_all_eo_files(path))
		{
			g_eo_install_path = path;
			return true;
		}
	}

	return false;
}

static bool try_hardcoded_paths()
{
	for (const char* path : hardcoded_install_paths)
	{
		if (try_hardcoded_path(path))
			return true;
	}

	return false;
}

static void save_custom_eo_path()
{
	HKEY registry;

	{
		int result = RegCreateKeyEx(
			HKEY_CURRENT_USER, "Software\\EOMap2", 0, 0, 0,
			KEY_READ | KEY_WRITE, 0, &registry, 0
		);

		if (result != ERROR_SUCCESS)
			return;
	}

	{
		int result = RegSetValueEx(registry, "EndlessDirectory", 0, REG_SZ,
			reinterpret_cast<const BYTE *>(g_eo_install_path.c_str()), g_eo_install_path.size());

		if (result != ERROR_SUCCESS)
		{
			RegCloseKey(registry);
			return;
		}
	}

	RegCloseKey(registry);
}

static bool select_eo_installation()
{
	ALLEGRO_FILECHOOSER* chooser = al_create_native_file_dialog(
		nullptr, "Select Endless Online directory", "",
		ALLEGRO_FILECHOOSER_FILE_MUST_EXIST | ALLEGRO_FILECHOOSER_FOLDER
	);

	while (true)
	{
		if (!al_show_native_file_dialog(nullptr, chooser))
		{
			return false;
		}

		const char* path = al_get_native_file_dialog_path(chooser, 0);

		if (!path || !has_all_eo_files(path))
		{
			al_show_native_message_box(nullptr, "Error", "Initialization error",
				"Selected directory does not contain a valid Endless Online installation", nullptr, ALLEGRO_MESSAGEBOX_ERROR);

			continue;
		}

		g_eo_install_path = path;
		break;
	}

	al_destroy_native_file_dialog(chooser);
	return true;
}

int main(int argc, char** argv)
{
	(void)argc;

	a5::_init();

	al_init_primitives_addon();

	if (!al_init_native_dialog_addon())
	{
		std::fputs("al_init_native_dialog_addon failed\n", stderr);
		return 1;
	}

	bool got_directory =
	    try_get_eo_registry_path("Software\\EOMap2", "EndlessDirectory")
	// || try_get_eo_registry_path("Software\\EndlessOnline", "Directory")
	 || try_hardcoded_paths();

	if (got_directory)
	{
		std::printf("Found install directory: %s\n", g_eo_install_path.c_str());
		std::fflush(stdout);
	}

	if (!got_directory)
	{
		if (!select_eo_installation())
		{
			al_show_native_message_box(nullptr, "Error", "Initialization error",
				"No valid Endless Online installation found", nullptr, ALLEGRO_MESSAGEBOX_ERROR);
			return 1;
		}

		save_custom_eo_path();
	}

	if (!PHYSFS_init(argv[0]))
	{
		al_show_native_message_box(nullptr, "Error", "Initialization error",
			"PHYSFS_init failed", nullptr, ALLEGRO_MESSAGEBOX_ERROR);
		return 1;
	}

	if (!PHYSFS_mountMemory(
		builtin_data, builtin_data_size, nullptr,
		"builtin.zip", nullptr, 0
	))
	{
		print_physfs_error();
		return 1;
	}

	al_set_physfs_file_interface();

	//al_register_assert_handler(ah);


	std::string map_filename;

	a5::Display map_display(640, 480, a5::Display::Windowed | a5::Display::Resizable | a5::Display::GenerateExposeEvents);
	map_display.Target();

	a5::Display pal_display(640, 320, a5::Display::Windowed | a5::Display::Resizable | a5::Display::GenerateExposeEvents);
	al_set_window_constraints(pal_display, 640, 0, 640, 0);
	pal_display.Target();

#ifdef WIN32
	GUI gui(map_display, pal_display);
#endif

	al_apply_window_constraints(pal_display, true);

	al_acknowledge_resize(map_display);
	al_acknowledge_resize(pal_display);

	ALLEGRO_EVENT_SOURCE* al_menu_source = al_get_default_menu_event_source();

	a5::Keyboard keyboard;
	a5::Mouse mouse;

	a5::Timer timer(60.0);
	a5::Timer anim_timer(2.0);

	std::string title("EO Map Editor 0.4.2 alpha");

	a5::Event_Queue q;

	bool running;

	al_set_new_bitmap_flags(
		ALLEGRO_FORCE_LOCKING
	);

	al_set_separate_blender(
		ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA,
		ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE
	);

	map_display.Target();
	a5::Font font(al_create_builtin_font());

	map_display.SetTitle(title.c_str());
	pal_display.SetTitle("Palette");

	EO_Map map;
	Map_Renderer map_renderer(map_display, font);
	Palette pal[10] = {3, 4, 5, 6, 6, 7, 3, 22, 5, -1};
	Pal_Renderer pal_renderer(pal_display);
	float map_window_scale = 1.0f;
	ALLEGRO_TRANSFORM identity_xform;
	ALLEGRO_TRANSFORM map_scale_xform;
	ALLEGRO_TRANSFORM map_scale_xform_inverse;

	float acc_map_sdx = 0.f;
	float acc_map_sdy = 0.f;

	al_identity_transform(&identity_xform);
	al_identity_transform(&map_scale_xform);

	al_copy_transform(&map_scale_xform_inverse, &map_scale_xform);
	al_invert_transform(&map_scale_xform_inverse);

#ifdef WIN32
	auto load_map = [&](const char *filename)
	{
		if (!filename)
		{
			Q_UNREGISTER_ALL()
			filename = gui.OpenFile();
			Q_REGISTER_ALL()
		}

		if (filename)
		{
			map_filename = filename;

			map_display.SetTitle((title + " - " + map_filename).c_str());

			gui.SetMenuEnabled(MENU_FILE_SAVE, true);
			gui.SetMenuEnabled(MENU_FILE_SAVE_AS, true);
			gui.SetMenuEnabled(MENU_MAP_PROPERTIES, true);

			map_display.Target();
			map.Load(filename);
		}
	};

	auto save_map = [&](const char *filename)
	{
		if (!filename)
		{
			Q_UNREGISTER_ALL()
			filename = gui.SaveFile();
			Q_REGISTER_ALL()
		}

		if (filename)
		{
			map_filename = filename;

			map_display.SetTitle((title + " - " + map_filename).c_str());

			map.Save(filename);
		}
	};
#endif
	auto show_hide_layer = [&](int layer)
	{
		map_renderer.show_layers[layer] = !map_renderer.show_layers[layer];
	};

	try
	{
		Q_REGISTER_ALL()

		anim_timer.Start();

		bool scroll_up = false, scroll_right = false, scroll_down = false, scroll_left = false;
		bool pal_scroll_up = false, pal_scroll_down = false;
		bool mouse_down = false;
		bool mouse_r_down = false;

		running = true;
		bool redraw = true;
		bool pal_redraw = true;

		int map_load_boost = 1000;
		int pal_load_boost = 1000;

		int pal_scrolled = -1;
		int map_scrolled = -1;

		map_display.Target();
		a5::Bitmap cursor(a5::Bitmap("cursor.bmp"));
		al_convert_mask_to_alpha(cursor, a5::Color(a5::RGB(255, 0, 255)));

		pal_display.Target();
		a5::Bitmap palhead(a5::Bitmap("palhead.bmp"));
		a5::Bitmap palfoot(a5::Bitmap("palfoot.bmp"));

		int mouse_tile_x = 0;
		int mouse_tile_y = 0;
		int mouse_inrange = false;

		int scroll_multiplier = 1;

		map_display.Target();
		map_renderer.SetMap(map);
		map_renderer.ResetView();

		pal_display.Target();
		pal_renderer.SetPal(0, pal[0]);
		pal_renderer.ResetView();

		bool map_drag_scroll = false;
		bool pal_drag_scroll = false;

		while (running)
		{
			std::unique_ptr<a5::Event> e;
#ifdef WIN32
			while (!gui.events.empty())
			{
				GUI::Event e = gui.events.back();
				gui.events.pop_back();

				switch (e.subtype)
				{
					case GUI::Event::Command:
						switch (e.resid)
						{
							case MENU_FILE_NEW:
								Q_UNREGISTER_ALL()
								gui.dialog_new_name[0] = '\0';
								gui.dialog_new_width = 0;
								gui.dialog_new_height = 0;
								if (gui.RunDialog(DIALOG_NEW_MAP))
								{
									EO_Map newmap;
									newmap.name = gui.dialog_new_name;
									newmap.width = gui.dialog_new_width - 1;
									newmap.height = gui.dialog_new_height - 1;
									newmap.loaded = true;
									map = newmap;
									map_renderer.ResetView();
									redraw = true;

									map_filename.clear();

									gui.SetMenuEnabled(MENU_FILE_SAVE, true);
									gui.SetMenuEnabled(MENU_FILE_SAVE_AS, true);
									gui.SetMenuEnabled(MENU_MAP_PROPERTIES, true);

									map_display.SetTitle((title + " - New Map").c_str());
								}
								Q_REGISTER_ALL()
								break;

							case MENU_FILE_OPEN:
								load_map(0);
								map_renderer.ResetView();
								map_load_boost = 1000;
								break;

							case MENU_FILE_SAVE:
								if (map_filename.empty())
									save_map(0);
								else
									save_map(map_filename.c_str());
								break;

							case MENU_FILE_SAVE_AS:
								save_map(0);
								break;

							case MENU_FILE_EXIT:
								running = false;
								break;

							case MENU_FILE_SELECT_EO_INSTALL:
							{
								std::string old_path = g_eo_install_path;

								if (select_eo_installation())
								{
									if (old_path != g_eo_install_path)
									{
										al_show_native_message_box(nullptr, "Success", "Endless Online Location Updated",
											"Please restart EOMap to re-load tiles from the new installation.", nullptr, 0);
										save_custom_eo_path();
									}
								}
							}
								break;

							case MENU_MAP_PROPERTIES:
								Q_UNREGISTER_ALL()
								gui.dialog_map_type          = unsigned(map.type);
								gui.dialog_map_effect        = unsigned(map.effect);
								strncpy(gui.dialog_map_name, map.name.c_str(), 24);
								gui.dialog_map_name[24] = '\0';
								gui.dialog_map_width         = map.width + 1;
								gui.dialog_map_height        = map.height + 1;
								gui.dialog_map_reset_x       = map.relog_x;
								gui.dialog_map_reset_y       = map.relog_y;
								gui.dialog_map_map_available = map.map_available != 0;
								gui.dialog_map_can_scroll    = map.can_scroll    != 0;
								gui.dialog_map_music_track   = map.music;
								gui.dialog_map_music_ambient = map.ambient_noise;
								gui.dialog_map_music_control = map.music_extra;
								if (gui.RunDialog(DIALOG_MAP_PROPERTIES))
								{
                                    map.type          = EO_Map::Type(gui.dialog_map_type);
                                    map.effect        = EO_Map::Effect(gui.dialog_map_effect);
                                    map.name          = gui.dialog_map_name;
                                    map.width         = gui.dialog_map_width - 1;
                                    map.height        = gui.dialog_map_height - 1;
                                    map.relog_x       = gui.dialog_map_reset_x;
                                    map.relog_y       = gui.dialog_map_reset_y;
                                    map.map_available = gui.dialog_map_map_available;
                                    map.can_scroll    = gui.dialog_map_can_scroll;
                                    map.music         = gui.dialog_map_music_track;
                                    map.ambient_noise = gui.dialog_map_music_ambient;
                                    map.music_extra   = gui.dialog_map_music_control;
                                    map.Cleanup();
								}
								Q_REGISTER_ALL()
								break;


							case MENU_LAYERS_GROUND:
							case MENU_LAYERS_OBJECTS:
							case MENU_LAYERS_OVERLAY:
							case MENU_LAYERS_DOWN_WALL:
							case MENU_LAYERS_RIGHT_WALL:
							case MENU_LAYERS_ROOF:
							case MENU_LAYERS_TOP:
							case MENU_LAYERS_SHADOW:
							case MENU_LAYERS_OVERLAY2:
							case MENU_LAYERS_SPAWNS:
							case MENU_LAYERS_SPECIAL:
							case MENU_LAYERS_GRID_LINES:
								Q_UNREGISTER_ALL()
								show_hide_layer(e.resid - MENU_LAYERS_GROUND);
								gui.SetMenuChecked(e.resid, map_renderer.show_layers[e.resid - MENU_LAYERS_GROUND]);
								Q_REGISTER_ALL()
								break;


							case MENU_HELP_ABOUT:
								Q_UNREGISTER_ALL()
								gui.RunDialog(DIALOG_ABOUT);
								Q_REGISTER_ALL()
								break;
						}
						break;
				}
			}
#endif // WIN32

			e = q.Wait();

			bool ack_resize = false;

			do
			{
				a5::Timer::Event *te;
				a5::Display::Event *de;
				a5::Keyboard::Event *ke;
				a5::Mouse::Event *me;

				if (e->has_raw && e->raw.type == ALLEGRO_EVENT_MENU_CLICK)
				{
					gui.events.push_back({GUI::Event::Command, WPARAM(e->raw.user.data1)});
				}
				else if ((te = dynamic_cast<a5::Timer::Event *>(e.get())))
				{
					if (te->source == &timer)
					{
						if (map.loaded)
						{
							if (scroll_up) { map_renderer.yoff -= 8 * scroll_multiplier; map_scrolled = 0; redraw = true; }
							if (scroll_right) { map_renderer.xoff += 8 * scroll_multiplier; map_scrolled = 0; redraw = true; }
							if (scroll_down) { map_renderer.yoff += 8 * scroll_multiplier; map_scrolled = 0; redraw = true; }
							if (scroll_left) { map_renderer.xoff -= 8 * scroll_multiplier; map_scrolled = 0; redraw = true; }
						}

						if (pal_scroll_up) { pal_renderer.yoff -= 8 * scroll_multiplier; pal_scrolled = 0; pal_redraw = true; }
						if (pal_scroll_down) { pal_renderer.yoff += 8 * scroll_multiplier; pal_scrolled = 0; pal_redraw = true; }

						pal_renderer.yoff = std::max(std::min(pal_renderer.yoff, pal_renderer.pal->height - pal_renderer.target.Height()), 0);

						if (pal_scrolled >= 0)
							pal_redraw = true;
					}
					else if (te->source == &anim_timer)
					{
						map_renderer.animation_state = (map_renderer.animation_state + 1) & 0x3;
						redraw = true;

						pal_renderer.animation_state = (pal_renderer.animation_state + 1) & 0x3;
						pal_redraw = true;
					}
				}
				else if ((de = dynamic_cast<a5::Display::Event *>(e.get())))
				{
					if (de->source == &map_display)
					{
						if (de->SubType() == a5::Display::Event::SwitchIn)
						{
							pal_redraw = true;
#ifdef WIN32
							gui.SwitchIn();
#endif // WIN32
						}
						else if (de->SubType() == a5::Display::Event::SwitchOut)
						{
#ifdef WIN32
							gui.SwitchOut();
#endif // WIN32
						}
						else if (de->SubType() == a5::Display::Event::Close)
						{
							running = false;
						}
						else if (de->SubType() == a5::Display::Event::Resize)
						{
							ack_resize = true;
							//mouse.SetRange(a5::Rectangle(0, 0, map_display.Width(), map_display.Height()));
							redraw = true;
						}
						else if (de->SubType() == a5::Display::Event::Expose)
						{
							redraw = true;
						}
					}
					else if (de->source == &pal_display)
					{
						if (de->SubType() == a5::Display::Event::Resize)
						{
							pal_display.AcknowledgeResize();
							//mouse.SetRange(a5::Rectangle(0, 0, map_display.Width(), map_display.Height()));
							pal_redraw = true;
						}
						else if (de->SubType() == a5::Display::Event::Expose)
						{
							pal_redraw = true;
						}
					}
				}
				else if ((ke = dynamic_cast<a5::Keyboard::Event *>(e.get())))
				{
					if (ke->display == map_display)
					{
						if (ke->SubType() == a5::Keyboard::Event::Down)
						{
							if (ke->keycode == a5::Keyboard::Key::Escape)
							{
								running = false;
							}
							else if (ke->keycode == a5::Keyboard::Key::F5)
							{
								redraw = true;
							}
							else if (ke->keycode == a5::Keyboard::Key::Up) scroll_up = true;
							else if (ke->keycode == a5::Keyboard::Key::Right) scroll_right = true;
							else if (ke->keycode == a5::Keyboard::Key::Down) scroll_down = true;
							else if (ke->keycode == a5::Keyboard::Key::Left) scroll_left = true;
						}
						else if (ke->SubType() == a5::Keyboard::Event::Up)
						{
							if (ke->keycode == a5::Keyboard::Key::Up) scroll_up = false;
							else if (ke->keycode == a5::Keyboard::Key::Right) scroll_right = false;
							else if (ke->keycode == a5::Keyboard::Key::Down) scroll_down = false;
							else if (ke->keycode == a5::Keyboard::Key::Left) scroll_left = false;
						}
					}
					else if (ke->display == pal_display)
					{
						if (ke->SubType() == a5::Keyboard::Event::Down)
						{
							if (ke->keycode == a5::Keyboard::Key::Escape)
							{
								// Hide window
							}
							else if (ke->keycode == a5::Keyboard::Key::F5)
							{
								pal_redraw = true;
							}
							else if (ke->keycode == a5::Keyboard::Key::Up)
                                pal_scroll_up = true;
							else if (ke->keycode == a5::Keyboard::Key::Down)
                                pal_scroll_down = true;
						}
						else if (ke->SubType() == a5::Keyboard::Event::Up)
						{
							if (ke->keycode == a5::Keyboard::Key::Up)
							{
							    pal_scroll_up = false;
                            }
							else if (ke->keycode == a5::Keyboard::Key::Down)
							{
							    pal_scroll_down = false;
                            }
						}
						else if (ke->SubType() == a5::Keyboard::Event::Char)
						{
							if (ke->keycode == a5::Keyboard::Key::PgUp)
							{
								pal_renderer.yoff -= pal_renderer.target.Height() / 2;
								pal_renderer.yoff = std::max(std::min(pal_renderer.yoff, pal_renderer.pal->height - pal_renderer.target.Height()), 0);
								pal_redraw = true;
								pal_scrolled = 0;
                            }
							else if (ke->keycode == a5::Keyboard::Key::PgDn)
							{
								pal_renderer.yoff += pal_renderer.target.Height() / 2;
								pal_renderer.yoff = std::max(std::min(pal_renderer.yoff, pal_renderer.pal->height - pal_renderer.target.Height()), 0);
								pal_redraw = true;
								pal_scrolled = 0;
                            }
							else if (ke->keycode == a5::Keyboard::Key::Home)
							{
								pal_renderer.yoff = 0;
								pal_redraw = true;
								pal_scrolled = 0;
                            }
							else if (ke->keycode == a5::Keyboard::Key::End)
							{
								pal_renderer.yoff = pal_renderer.pal->height - pal_renderer.target.Height();
								pal_redraw = true;
								pal_scrolled = 0;
                            }
						}
					}

					if (ke->keycode == a5::Keyboard::Key::LeftCtrl || ke->keycode == a5::Keyboard::Key::RightCtrl)
					{
						if (ke->SubType() == a5::Keyboard::Event::Down)
						{
							scroll_multiplier = 2;
						}
						else if (ke->SubType() == a5::Keyboard::Event::Up)
						{
							scroll_multiplier = 1;
						}
					}
				}
				else if ((me = dynamic_cast<a5::Mouse::Event *>(e.get())))
				{
					if (map.loaded && me->display == map_display)
					{
						if (mouse_inrange && me->SubType() == a5::Mouse::Event::Down)
						{
							if (me->button == a5::Mouse::Left)
							{
								mouse_down = true;
								if (pal_renderer.pal->layer < 9)
								{
									EO_Map::SetTile(map.gfxrows[pal_renderer.pal->layer], pal_renderer.pal->selected_tile, mouse_tile_x, mouse_tile_y);
								}
								else
								{
#ifdef WIN32
									if (pal_renderer.pal->selected_tile == 37)
									{
										Q_UNREGISTER_ALL()

										EO_Map::Warp *warp = map.GetWarpTile(mouse_tile_x, mouse_tile_y);

										if (warp)
										{
											gui.dialog_warp_map = warp->warp_map;
											gui.dialog_warp_x = warp->warp_x;
											gui.dialog_warp_y = warp->warp_y;
											gui.dialog_warp_level = warp->level;
											gui.dialog_warp_door = warp->door != EO_Map::NoDoor;
											gui.dialog_warp_key = warp->door ? (warp->door - 1) : 0;
										}
										else
										{
											gui.dialog_warp_map = 0;
											gui.dialog_warp_x = 0;
											gui.dialog_warp_y = 0;
											gui.dialog_warp_level = 0;
											gui.dialog_warp_door = 0;
											gui.dialog_warp_key = 0;
										}

										if (gui.RunDialog(DIALOG_EDIT_WARP))
										{
											map.SetTileWarp(gui.dialog_warp_map, gui.dialog_warp_x, gui.dialog_warp_y, gui.dialog_warp_level, gui.dialog_warp_door ? (1 + gui.dialog_warp_key) : EO_Map::NoDoor, mouse_tile_x, mouse_tile_y);
										}

										Q_REGISTER_ALL()
										mouse_down = false;
									}
									else if (pal_renderer.pal->selected_tile == 38)
									{
										Q_UNREGISTER_ALL()

										std::vector<EO_Map::Chest> chest = map.GetChestSpawns(mouse_tile_x, mouse_tile_y);
										gui.chest_spawns = chest;

                                        if (gui.RunDialog(DIALOG_ITEM_SPAWN_LIST))
                                        {
                                            do
                                            {
                                                switch (gui.dialog_item_spawn_action)
                                                {
                                                    case 1: // Adding item spawn
                                                    {
                                                        if (gui.RunDialog(DIALOG_EDIT_ITEM_SPAWN))
                                                        {
                                                            gui.dialog_edited_item_spawn.x = mouse_tile_x;
                                                            gui.dialog_edited_item_spawn.y = mouse_tile_y;
                                                            map.chests.push_back(gui.dialog_edited_item_spawn);
                                                        }
                                                    }
                                                    break;

                                                    case 2: // Deleting item spawn
                                                    {
                                                        map.DelChestSpawn(gui.dialog_current_item_spawn);
                                                    }
                                                    break;

                                                    case 3: // Editing item spawn
                                                    {
                                                        if (gui.RunDialog(DIALOG_EDIT_ITEM_SPAWN))
                                                        {
                                                            EO_Map::Chest *editing = map.GetChestSpawn(gui.dialog_current_item_spawn);

                                                            editing->key    = gui.dialog_edited_item_spawn.key;
                                                            editing->slot   = gui.dialog_edited_item_spawn.slot;
                                                            editing->item   = gui.dialog_edited_item_spawn.item;
                                                            editing->time   = gui.dialog_edited_item_spawn.time;
                                                            editing->amount = gui.dialog_edited_item_spawn.amount;
                                                        }
                                                    }
                                                    break;
                                                }
                                                chest = map.GetChestSpawns(mouse_tile_x, mouse_tile_y);

                                                gui.chest_spawns = chest;
                                            }
                                            while (gui.RunDialog(DIALOG_ITEM_SPAWN_LIST));
                                        }
										Q_REGISTER_ALL()

										mouse_down = false;
									}
									else if (pal_renderer.pal->selected_tile == 39)
									{
										Q_UNREGISTER_ALL()

										std::vector<EO_Map::NPC> npc = map.GetNPCSpawns(mouse_tile_x, mouse_tile_y);
										gui.npc_spawns = npc;

                                        if (gui.RunDialog(DIALOG_NPC_SPAWN_LIST))
                                        {
                                            do
                                            {
                                                switch (gui.dialog_npc_spawn_action)
                                                {
                                                    case 1: // Adding item spawn
                                                    {
                                                        if (gui.RunDialog(DIALOG_EDIT_NPC_SPAWN))
                                                        {
                                                            gui.dialog_edited_npc_spawn.x = mouse_tile_x;
                                                            gui.dialog_edited_npc_spawn.y = mouse_tile_y;
                                                            map.npcs.push_back(gui.dialog_edited_npc_spawn);
                                                        }
                                                    }
                                                    break;

                                                    case 2: // Deleting npc spawn
                                                    {
                                                        map.DelNPCSpawn(gui.dialog_current_npc_spawn);
                                                    }
                                                    break;

                                                    case 3: // Editing npc spawn
                                                    {
                                                        if (gui.RunDialog(DIALOG_EDIT_NPC_SPAWN))
                                                        {
                                                            EO_Map::NPC *editing = map.GetNPCSpawn(gui.dialog_current_npc_spawn);

                                                            editing->spawn_type = gui.dialog_edited_npc_spawn.spawn_type;
                                                            editing->spawn_time = gui.dialog_edited_npc_spawn.spawn_time;
                                                            editing->amount     = gui.dialog_edited_npc_spawn.amount;
                                                        }
                                                    }
                                                    break;
                                                }
                                                npc = map.GetNPCSpawns(mouse_tile_x, mouse_tile_y);

                                                gui.npc_spawns = npc;
                                            }
                                            while (gui.RunDialog(DIALOG_NPC_SPAWN_LIST));
                                        }
										Q_REGISTER_ALL()

										mouse_down = false;
									}
									else if (pal_renderer.pal->selected_tile == 40)
									{
										Q_UNREGISTER_ALL()
										EO_Map::Sign *sign = map.GetSign(mouse_tile_x, mouse_tile_y);

										if (sign)
										{
											gui.dialog_sign_title = sign->title;
                                            gui.dialog_sign_message = sign->message;
										}
										else
										{
											gui.dialog_sign_title = "Sign title";
                                            gui.dialog_sign_message = "Sign message.";
										}

                                        if (gui.RunDialog(DIALOG_EDIT_SIGN))
                                        {
                                            map.SetTileSign(gui.dialog_sign_title, gui.dialog_sign_message, mouse_tile_x, mouse_tile_y);
                                        }
										Q_REGISTER_ALL()

										mouse_down = false;
									}
									else
#endif // WIN32
									{
										map.SetTileSpec(EO_Map::Tile_Spec(pal_renderer.pal->selected_tile), mouse_tile_x, mouse_tile_y);
									}
								}
								redraw = true;
							}
							else if (me->button == a5::Mouse::Right)
							{
								mouse_r_down = true;
								if (pal_renderer.pal->layer < 9)
								{
									EO_Map::DelTile(map.gfxrows[pal_renderer.pal->layer], mouse_tile_x, mouse_tile_y);
								}
								else
								{
									map.DelTileSpec(mouse_tile_x, mouse_tile_y);
								}
								redraw = true;
							}
							else if (me->SubType() == a5::Mouse::Event::Down)
							{
								map_drag_scroll = true;
							}
						}
						else if (me->SubType() == a5::Mouse::Event::Down && me->button == a5::Mouse::Middle)
						{
							map_drag_scroll = true;
						}
						else if (me->SubType() == a5::Mouse::Event::Move)
						{
							float sx = me->x;
							float sy = me->y;
							al_transform_coordinates(&map_scale_xform_inverse, &sx, &sy);
							me->x = sx;
							me->y = sy;

							int ymouse = ((((me->y + map_renderer.yoff + 16) << 1) - ((me->x + map_renderer.xoff))) >> 1) & INT_MAX;
							int xmouse = ((me->x + map_renderer.xoff) + ymouse) & INT_MAX;
							int new_mouse_tile_y = (ymouse >> 5);
							int new_mouse_tile_x = (xmouse >> 5) - 1;

							if (map_drag_scroll)
							{
								float sdx = me->dx * (1.f / map_window_scale);
								float sdy = me->dy * (1.f / map_window_scale);

								acc_map_sdx += sdx;
								acc_map_sdy += sdy;

								map_scrolled = 0;
								redraw = true;

								if (abs(acc_map_sdx) >= 1.f)
								{
									int sdx_i = floor(acc_map_sdx);
									map_renderer.xoff -= sdx_i * scroll_multiplier;
									acc_map_sdx -= sdx_i;
								}

								if (abs(acc_map_sdy) >= 1.f)
								{
									int sdy_i = floor(acc_map_sdy);
									map_renderer.yoff -= sdy_i * scroll_multiplier;
									acc_map_sdy -= sdy_i;
								}
							}

							if (new_mouse_tile_y != mouse_tile_y || new_mouse_tile_x != mouse_tile_x)
							{
								mouse_tile_y = new_mouse_tile_y;
								mouse_tile_x = new_mouse_tile_x;

								mouse_inrange = (mouse_tile_x >= 0 && mouse_tile_y >= 0 && ((mouse_tile_x <= map.width && mouse_tile_y <= map.height) || map.HasSomething(mouse_tile_x, mouse_tile_y)));

								if (mouse_inrange)
								{
									if (mouse_down)
									{
										if (pal_renderer.pal->layer < 9)
										{
											EO_Map::SetTile(map.gfxrows[pal_renderer.pal->layer], pal_renderer.pal->selected_tile, mouse_tile_x, mouse_tile_y);
										}
										else if (pal_renderer.pal->selected_tile != 37)
										{
											map.SetTileSpec(EO_Map::Tile_Spec(pal_renderer.pal->selected_tile), mouse_tile_x, mouse_tile_y);
										}
									}

									if (mouse_r_down)
									{
										if (pal_renderer.pal->layer < 9)
										{
											EO_Map::DelTile(map.gfxrows[pal_renderer.pal->layer], mouse_tile_x, mouse_tile_y);
										}
										else
										{
											map.DelTileSpec(mouse_tile_x, mouse_tile_y);
										}
									}
								}

								redraw = true;
							}

							ALLEGRO_KEYBOARD_STATE kstate;
							al_get_keyboard_state(&kstate);
							bool ctrl = al_key_down(&kstate, ALLEGRO_KEY_LCTRL) || al_key_down(&kstate, ALLEGRO_KEY_RCTRL);

							if (ctrl && me->dz) {
								while (me->dz > 0 && map_window_scale < 2.0f)
								{
									--me->dz;
									map_window_scale += 0.1f;
									map_renderer.xoff += map_display.Width() * 0.05f;
									map_renderer.yoff += map_display.Height() * 0.05f;
								}
								while (me->dz < 0 && map_window_scale > 0.5f)
								{
									++me->dz;
									map_window_scale -= 0.1f;
									map_renderer.xoff -= map_display.Width() * 0.05f;
									map_renderer.yoff -= map_display.Height() * 0.05f;
								}

								if (map_window_scale >= 0.99f && map_window_scale <= 1.01f)
									map_window_scale = 1.0f;

								if (map_window_scale < 0.5f)
									map_window_scale = 0.5f;

								if (map_window_scale > 2.0f)
									map_window_scale = 2.0f;

								al_identity_transform(&map_scale_xform);

								al_scale_transform(&map_scale_xform, map_window_scale, map_window_scale);

								al_copy_transform(&map_scale_xform_inverse, &map_scale_xform);
								al_invert_transform(&map_scale_xform_inverse);

								map_display.Target();
								map_renderer.RebuildTarget(
									map_display.Width() / map_window_scale,
									map_display.Height() / map_window_scale
								);

								redraw = true;
							}
						}
						else if (me->SubType() == a5::Mouse::Event::Up)
						{
							if (me->button == a5::Mouse::Left)
							{
								mouse_down = false;
							}
							else if (me->button == a5::Mouse::Right)
							{
								mouse_r_down = false;
							}
							else if (me->button == a5::Mouse::Middle)
							{
								map_drag_scroll = false;
							}
						}
					}
					else if (me->display == pal_display)
					{
						if (me->SubType() == a5::Mouse::Event::Down)
						{
							if (me->button == a5::Mouse::Left)
							{
								if (me->y < 32)
								{
									for (int i = 0; i < 8; ++i)
									{
										if (me->x > i * 80 && me->x < (i + 1) * 80)
										{
											pal_display.Target();
											map_renderer.highlight_spec = false;
											pal_renderer.SetPal(i, pal[i]);
											pal_load_boost = 1000;
											break;
										}
									}
								}
								else if (me->y > pal_display.Height() - 32)
								{
									for (int i = 0; i < 2; ++i)
									{
										if (me->x > i * 80 && me->x < (i + 1) * 80)
										{
											pal_display.Target();
											map_renderer.highlight_spec = (i == 1);
											pal_renderer.SetPal(8+i, pal[8+i]);
											pal_load_boost = 1000;
											break;
										}
									}
								}
								else
								{
									pal_renderer.pal->Click(me->x, pal_renderer.yoff + me->y);
								}

								pal_redraw = true;
							}
							else if (me->button == a5::Mouse::Middle)
							{
								pal_drag_scroll = true;
							}
							else if (me->button == a5::Mouse::Right)
							{
								if (pal_renderer.pal->layer == 0)
								{
									map.fill_tile = pal_renderer.pal->RightClick(me->x, pal_renderer.yoff + me->y);
									redraw = true;
			 					}
							}
						}
						else if (me->SubType() == a5::Mouse::Event::Up)
						{
							if (me->button == a5::Mouse::Middle)
							{
								pal_drag_scroll = false;
							}
						}
						else if (me->SubType() == a5::Mouse::Event::Move)
						{
							if (pal_drag_scroll)
							{
								pal_scrolled = 0;
								pal_redraw = true;
								pal_renderer.yoff -= me->dy * scroll_multiplier;
								pal_renderer.yoff = std::max(std::min(pal_renderer.yoff, pal_renderer.pal->height - pal_renderer.target.Height()), 0);
							}

							if (me->dz)
							{
								pal_scrolled = 0;
								pal_redraw = true;
								pal_renderer.yoff -= me->dz * 96 * scroll_multiplier;
								pal_renderer.yoff = std::max(std::min(pal_renderer.yoff, pal_renderer.pal->height - pal_renderer.target.Height()), 0);
							}
						}
					}
				}
			} while ((e = q.Get()));

			if (ack_resize)
			{
				map_display.Target();
				al_acknowledge_resize(map_display);
				map_renderer.RebuildTarget(map_display.Width() / map_window_scale, map_display.Height() / map_window_scale);
			}

			if (redraw)
			{
				map_renderer.target.Target();
				a5::disable_auto_target = true;
				map_renderer.target.Clear();

				map_renderer.gfxloader.frame_load_allocation = 100 + map_load_boost;
				map_load_boost = 0;
				if (map.loaded)
				{
					al_hold_bitmap_drawing(true);

					map_renderer.Render();

					if (mouse_inrange)
					{
						int mouse_draw_x = (mouse_tile_x << 5) - (mouse_tile_y << 5) - map_renderer.xoff;
						int mouse_draw_y = (mouse_tile_x << 4) + (mouse_tile_y << 4) - map_renderer.yoff;
						map_renderer.target.Blit(cursor, mouse_draw_x, mouse_draw_y);
					}

					al_hold_bitmap_drawing(false);
				}

				map_display.Target();

				map_display.Clear();
				al_use_transform(&map_scale_xform);
				map_display.Blit(map_renderer.target, 0, 0);
				al_use_transform(&identity_xform);

				if (map.loaded && mouse_inrange)
				{
					al_draw_textf(
						font, a5::Color(a5::RGB(255, 255, 255)),
						map_display.Width()-20 - (8*int(log10(mouse_tile_y))),
						map_display.Height()-12,
						ALLEGRO_ALIGN_RIGHT, "%ix%i", mouse_tile_x, mouse_tile_y
					);
				}

				if (map_window_scale != 1.f)
					al_draw_textf(
						font, a5::Color(a5::RGB(255, 255, 255)),
						map_display.Width()-20,
						map_display.Height()-22,
						ALLEGRO_ALIGN_RIGHT, "Zoom: %d%%", (int)(map_window_scale * 100.f + 0.25f)
					);

				map_display.Flip();
				a5::disable_auto_target = false;

				if (map_renderer.gfxloader.frame_load_allocation > 0)
					redraw = false;
			}

			if (pal_redraw)
			{
				pal_renderer.target.Target();
				a5::disable_auto_target = true;

				if (pal_renderer.pal->layer == 7) pal_display.Clear(a5::RGB(128, 128, 128));
				else pal_display.Clear(a5::RGB(0, 0, 0));

				pal_renderer.gfxloader.frame_load_allocation = 20 + pal_load_boost;
				pal_load_boost = 0;
				pal_renderer.Render();

				pal_display.Blit(palhead, 0, 0);
				pal_display.Blit(palfoot, 0, pal_display.Height() - 32);

				if (pal_renderer.pal->layer < 8)
				{
					pal_display.BlitTinted(palhead, a5::RGBA(160, 160, 160, 255), pal_renderer.pal->layer * 80, 0, a5::Rectangle(pal_renderer.pal->layer * 80, 0, (pal_renderer.pal->layer + 1) * 80, 32));
				}
				else
				{
					pal_display.BlitTinted(palfoot, a5::RGBA(160, 160, 160, 255), (pal_renderer.pal->layer - 8) * 80, pal_display.Height() - 32, a5::Rectangle((pal_renderer.pal->layer - 8) * 80, 0, (pal_renderer.pal->layer - 7) * 80, 32));
				}

				if (pal_scrolled != -1)
				{
					if (pal_scrolled == 0)
						timer.Start();

					int alpha = 220 - (++pal_scrolled * 2);
					if (pal_scrolled < 30) alpha = 160;

					float f = pal_renderer.yoff;
					f /= pal_renderer.pal->height - pal_display.Height();
					f *= pal_display.Height() - 104;

					al_draw_line(634, f + 36, 634, f + 66, a5::Color(a5::RGBA(255, 255, 255, alpha)), 4.0f);

					if (alpha <= 0)
					{
						pal_scrolled = -1;
						timer.Stop();
					}
				}

				if (pal_renderer.gfxloader.frame_load_allocation > 0)
					pal_redraw = false;

				pal_renderer.target.Flip();
				a5::disable_auto_target = false;
			}
		}
	}
	catch (EOMap_Exception& e)
	{
		fprintf(stderr, "eomap exception: %s\n", e.message());
		al_show_native_message_box(nullptr, "Error", "Uncaught EOMap exception", e.message(), nullptr, ALLEGRO_MESSAGEBOX_ERROR);
	}
	catch (std::exception& e)
	{
		fprintf(stderr, "std exception: %s\n", e.what());
		al_show_native_message_box(nullptr, "Error", "Uncaught exception", e.what(), nullptr, ALLEGRO_MESSAGEBOX_ERROR);
	}

	return 0;
}
