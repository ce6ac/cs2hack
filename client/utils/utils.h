#include "vector.h"

Vector3 screenSize(1920.f, 1080.f, 0); // default

struct view_matrix_t {
    float matrix[4][4];
};

std::string sanitize_utf8(const std::string& input) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	try {
		std::wstring wide_str = converter.from_bytes(input);
		return converter.to_bytes(wide_str);
	} catch (const std::range_error&) {
		return "";
	}
}

inline bool WorldToScreen(const Vector3& worldPos, Vector3& screenPos, const view_matrix_t& viewMatrix)
{
	float clip_x = worldPos.x * viewMatrix.matrix[0][0] + worldPos.y * viewMatrix.matrix[0][1] + worldPos.z * viewMatrix.matrix[0][2] + viewMatrix.matrix[0][3];
	float clip_y = worldPos.x * viewMatrix.matrix[1][0] + worldPos.y * viewMatrix.matrix[1][1] + worldPos.z * viewMatrix.matrix[1][2] + viewMatrix.matrix[1][3];
	float clip_z = worldPos.x * viewMatrix.matrix[2][0] + worldPos.y * viewMatrix.matrix[2][1] + worldPos.z * viewMatrix.matrix[2][2] + viewMatrix.matrix[2][3];
	float clip_w = worldPos.x * viewMatrix.matrix[3][0] + worldPos.y * viewMatrix.matrix[3][1] + worldPos.z * viewMatrix.matrix[3][2] + viewMatrix.matrix[3][3];

	if (clip_w < 0.1f) // To prevent division by zero and culling objects behind the camera
		return false;

	// Normalize to NDC (Normalized Device Coordinates)
	float ndc_x = clip_x / clip_w;
	float ndc_y = clip_y / clip_w;

	// Convert to screen coordinates
	screenPos.x = (screenSize.x / 2 * ndc_x) + (ndc_x + 1440 / 2);
	screenPos.y = -(screenSize.y / 2 * ndc_y) + (ndc_y + 1080 / 2);

	return true;
}