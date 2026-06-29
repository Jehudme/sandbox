#include <flecs.h>
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#pragma message(TOSTRING(ECS_COMPONENT_DECLARE(my_struct)))
int main() {}
