#pragma once
enum class CollisionLocation
{
	kNone = 0,
	kLeft = 1 << 0,
	kTop = 1 << 1,
	kRight = 1 << 2,
	kBottom = 1 << 3,

	kXAxis = kLeft | kRight,
	kYAxis = kTop | kBottom,
};