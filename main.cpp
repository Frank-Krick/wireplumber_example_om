#include <glib-object.h>
#include <iostream>
#include <pipewire/keys.h>
#include <thread>
#include <wp/wp.h>

struct WirePlumberControl {
  GMainLoop *loop = nullptr;
  WpCore *core = nullptr;
  WpObjectManager *om = nullptr;
};

int main(int argc, char *argv[]) {
  wp_init(WP_INIT_ALL);
  WirePlumberControl wire_plumber_service;

  wire_plumber_service.loop = g_main_loop_new(nullptr, false);
  wire_plumber_service.core = wp_core_new(nullptr, nullptr, nullptr);
  if (wp_core_connect(wire_plumber_service.core)) {
    wire_plumber_service.om = wp_object_manager_new();
    wp_object_manager_add_interest(wire_plumber_service.om, WP_TYPE_PORT,
                                   nullptr);
    wp_object_manager_request_object_features(
        wire_plumber_service.om, WP_TYPE_GLOBAL_PROXY,
        WP_PIPEWIRE_OBJECT_FEATURES_MINIMAL);

    wp_core_install_object_manager(wire_plumber_service.core,
                                   wire_plumber_service.om);

    g_signal_connect(
        wire_plumber_service.om, "object-added",
        G_CALLBACK(+[](WpObjectManager *object_manager, gpointer object,
                       gpointer user_data) {
          const auto g_object = static_cast<WpPipewireObject *>(object);

          const auto id = std::atoi(
              wp_pipewire_object_get_property(g_object, PW_KEY_PORT_ID));
          const auto name = std::string(
              wp_pipewire_object_get_property(g_object, PW_KEY_PORT_NAME));
          const auto alias = std::string(
              wp_pipewire_object_get_property(g_object, PW_KEY_PORT_ALIAS));

          std::cout << "Object added, id " << id << ", name " << name
                    << ", alias " << alias << std::endl;
        }),
        nullptr);

    g_main_loop_run(wire_plumber_service.loop);
  }
}
