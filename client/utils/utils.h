#include <random>
#include "vector.h"
#include "../game.h"

Vector3 screen_size(1920.f, 1080.f, 0); // default, most common res ?

void seed_random() {
    std::srand(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count() % UINT_MAX
    ));
}

int random_int(int min, int max) {
    seed_random();
    return min + (rand() % (max - min + 1));
}

float random_float(float min, float max) {
    seed_random();
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) / (max - min));
}

std::string sanitize_utf8(const std::string& input) {
	try {
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
		std::wstring wide_str = converter.from_bytes(input);
		return converter.to_bytes(wide_str);
	} catch (const std::range_error&) {
		return "";
	}
}

// from uc or smth idk
inline bool world_to_screen(const Vector3& worldPos, Vector3& screenPos, const view_matrix_t& viewMatrix)
{
	float clip_x = worldPos.x * viewMatrix.matrix[0][0] + worldPos.y * viewMatrix.matrix[0][1] + worldPos.z * viewMatrix.matrix[0][2] + viewMatrix.matrix[0][3];
	float clip_y = worldPos.x * viewMatrix.matrix[1][0] + worldPos.y * viewMatrix.matrix[1][1] + worldPos.z * viewMatrix.matrix[1][2] + viewMatrix.matrix[1][3];
	float clip_z = worldPos.x * viewMatrix.matrix[2][0] + worldPos.y * viewMatrix.matrix[2][1] + worldPos.z * viewMatrix.matrix[2][2] + viewMatrix.matrix[2][3];
	float clip_w = worldPos.x * viewMatrix.matrix[3][0] + worldPos.y * viewMatrix.matrix[3][1] + worldPos.z * viewMatrix.matrix[3][2] + viewMatrix.matrix[3][3];

	if (clip_w < 0.1f)
		return false;

	float ndc_x = clip_x / clip_w;
	float ndc_y = clip_y / clip_w;

	screenPos.x = (screen_size.x / 2 * ndc_x) + (ndc_x + 1440 / 2);
	screenPos.y = -(screen_size.y / 2 * ndc_y) + (ndc_y + 1080 / 2);

	return true;
}