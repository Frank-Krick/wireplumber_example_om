#include "glib-object.h"
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <wp/wp.h>

struct Port {
  std::string alias;
};

std::vector<Port> iterate(WpObjectManager *om) {
  std::vector<Port> result;
  const auto om_iter = wp_object_manager_new_iterator(om);
  GValue port_value;
  while (wp_iterator_next(om_iter, &port_value)) {
    /*
    if (port_value.g_type != G_TYPE_OBJECT) {
      std::cout << "Not an object" << std::endl;
    }

    if (port_value.g_type != WP_TYPE_PORT) {
      std::cout << "Not a port" << std::endl;
    }
    */

    std::cout << g_type_name(port_value.g_type) << std::endl;
    /*
        auto object = g_value_get_object(&port_value);
        std::cout << "object: " << object << std::endl;
        if (object == nullptr) {
          continue;
        }

        WpPort *port = WP_PORT(object);
        const auto property_value = wp_pipewire_object_get_property(
            reinterpret_cast<WpPipewireObject *>(port), "port.alias");
        result.push_back(Port{.alias = std::string(property_value)});
        */
    g_value_unset(&port_value);
  }
  return result;
}

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
        wire_plumber_service.om, "installed",
        G_CALLBACK(+[](WpObjectManager *object_manager, gpointer user_data) {
          iterate(object_manager);
        }),
        nullptr);

    std::thread invoker([&wire_plumber_service]() {
      std::this_thread::sleep_for(std::chrono::seconds(10));
      iterate(wire_plumber_service.om);
      std::cout << "Invoked" << std::endl;

      iterate(wire_plumber_service.om);
      std::cout << "Invoked" << std::endl;
    });

    g_main_loop_run(wire_plumber_service.loop);
  }
}
