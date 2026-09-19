#include "player.h"

#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/core/math.hpp>

using namespace godot;

Player::Player() {}

Player::~Player() {}

void Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Player::set_speed);
	ClassDB::bind_method(D_METHOD("get_speed"), &Player::get_speed);
	ClassDB::add_property("Player", PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

	ClassDB::bind_method(D_METHOD("set_jump_velocity", "jump_velocity"), &Player::set_jump_velocity);
	ClassDB::bind_method(D_METHOD("get_jump_velocity"), &Player::get_jump_velocity);
	ClassDB::add_property("Player", PropertyInfo(Variant::FLOAT, "jump_velocity"), "set_jump_velocity", "get_jump_velocity");

	ClassDB::bind_method(D_METHOD("set_mouse_sensitivity", "sensitivity"), &Player::set_mouse_sensitivity);
	ClassDB::bind_method(D_METHOD("get_mouse_sensitivity"), &Player::get_mouse_sensitivity);
	ClassDB::add_property("Player", PropertyInfo(Variant::FLOAT, "mouse_sensitivity"), "set_mouse_sensitivity", "get_mouse_sensitivity");
}

void Player::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	camera_pivot = Object::cast_to<Node3D>(get_node_or_null(NodePath("CameraPivot")));
	Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
}

void Player::_physics_process(double p_delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Vector3 velocity = get_velocity();

	if (!is_on_floor()) {
		velocity.y -= gravity * p_delta;
	}

	if (Input::get_singleton()->is_action_just_pressed("jump") && is_on_floor()) {
		velocity.y = jump_velocity;
	}

	Vector2 input_dir = Input::get_singleton()->get_vector("move_left", "move_right", "move_forward", "move_back");
	Vector3 direction = get_transform().basis.xform(Vector3(input_dir.x, 0, input_dir.y)).normalized();

	if (direction.length_squared() > 0.0) {
		velocity.x = direction.x * speed;
		velocity.z = direction.z * speed;
	} else {
		velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)speed);
		velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)speed);
	}

	set_velocity(velocity);
	move_and_slide();
}

void Player::_unhandled_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseMotion> mouse_motion = p_event;
	if (mouse_motion.is_valid() && Input::get_singleton()->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
		Vector2 relative = mouse_motion->get_relative();

		rotate_y(-relative.x * mouse_sensitivity);

		if (camera_pivot != nullptr) {
			Vector3 pivot_rotation = camera_pivot->get_rotation();
			pivot_rotation.x -= relative.y * mouse_sensitivity;
			pivot_rotation.x = CLAMP(pivot_rotation.x, Math::deg_to_rad(-89.0), Math::deg_to_rad(89.0));
			camera_pivot->set_rotation(pivot_rotation);
		}
	}
}

void Player::set_speed(double p_speed) {
	speed = p_speed;
}

double Player::get_speed() const {
	return speed;
}

void Player::set_jump_velocity(double p_jump_velocity) {
	jump_velocity = p_jump_velocity;
}

double Player::get_jump_velocity() const {
	return jump_velocity;
}

void Player::set_mouse_sensitivity(double p_sensitivity) {
	mouse_sensitivity = p_sensitivity;
}

double Player::get_mouse_sensitivity() const {
	return mouse_sensitivity;
}
