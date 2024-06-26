#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/quaternion.hpp>


#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <chrono>
#include <vector>
#include <fstream>
#include <array>
#include <optional>
#include <set>
#include <unordered_map>
#include <thread>
#include <ctime>
#include <string>
#include <functional>


#define USE_VULKAN 1
#define USE_DIRECT_X 0
