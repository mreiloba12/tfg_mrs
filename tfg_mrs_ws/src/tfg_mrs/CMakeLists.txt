# Versión mínima de CMake necesaria
 cmake_minimum_required(VERSION 3.8)
 # Nombre del proyecto
 project(tfg_mrs)
 # DEPENDENCIAS DE ROS 2 NECESARIAS PARA COMPILAR NODOS EN C
 find_package(ament_cmake REQUIRED)
 find_package(rcl REQUIRED)
 find_package(rclc REQUIRED)
 # DEFINICION DEL NODO CONTROL DIRECCION
 add_executable(control_direccion src/control_direccion.c)
 target_include_directories(control_direccion PUBLIC
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
 $<INSTALL_INTERFACE:include>)
 ament_target_dependencies(control_direccion rcl rclc)
 # INSTALACION DEL EJECUTABLE
 install(TARGETS
 control_direccion
 DESTINATION lib/${PROJECT_NAME})
 # FINALIZACIÓN DEL PAQUETE
 ament_package()
