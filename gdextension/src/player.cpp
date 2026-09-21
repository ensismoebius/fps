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

	ClassDB::bind_method(D_METHOD("set_push_force", "push_force"), &Player::set_push_force);
	ClassDB::bind_method(D_METHOD("get_push_force"), &Player::get_push_force);
	ClassDB::add_property("Player", PropertyInfo(Variant::FLOAT, "push_force"), "set_push_force", "get_push_force");
}

void Player::_ready() {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	camera_pivot = Object::cast_to<Node3D>(get_node_or_null(NodePath("CameraPivot")));
	Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);

	// game/main.tscn declares metadata/mass on this node (see tutorial 05b);
	// read it once here so the push force below scales with it.
	if (has_meta("mass")) {
		player_mass = (double)get_meta("mass");
	}
}

void Player::_physics_process(double p_delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Vector3 velocity = get_velocity();
	bool was_on_floor = is_on_floor();

	if (!is_on_floor()) {
		velocity.y -= gravity * p_delta;
	}

	if (Input::get_singleton()->is_action_just_pressed("jump") && is_on_floor()) {
		velocity.y = jump_velocity;
	}

	Vector2 input_dir = Input::get_singleton()->get_vector("move_left", "move_right", "move_forward", "move_back");
	Vector3 direction = get_transform().basis.xform(Vector3(input_dir.x, 0, input_dir.y)).normalized();

	// Traction scales with the friction of whatever we stood on last tick
	// (current_floor_friction, sampled below): concrete (0.8) grips almost
	// instantly, metal (0.4) takes noticeably longer to speed up or stop.
	real_t traction = (real_t)speed * (real_t)Math::clamp(current_floor_friction, 0.15, 1.0);

	if (direction.length_squared() > 0.0) {
		velocity.x = Math::move_toward(velocity.x, direction.x * (real_t)speed, traction);
		velocity.z = Math::move_toward(velocity.z, direction.z * (real_t)speed, traction);
	} else {
		velocity.x = Math::move_toward(velocity.x, (real_t)0.0, traction);
		velocity.z = Math::move_toward(velocity.z, (real_t)0.0, traction);
	}

	real_t impact_velocity_y = velocity.y;

	set_velocity(velocity);
	move_and_slide();

	// Sample the surface we're touching now (used immediately below for the
	// impact bounce, and again next tick for traction — see current_floor_friction
	// above). This must run right after move_and_slide(), before the bounce
	// check: on the very tick we land, get_slide_collision() first becomes
	// non-empty here, so reading it later (i.e. leaving last tick's stale
	// "still airborne" values in place) would mean bounce never fires on the
	// tick it actually should.
	current_floor_friction = 1.0;
	current_floor_bounce = 0.0;

	for (int i = 0; i < get_slide_collision_count(); i++) {
		Ref<KinematicCollision3D> collision = get_slide_collision(i);
		if (collision->get_normal().y < 0.7) {
			continue; // A wall or ceiling hit, not the floor.
		}

		Ref<PhysicsMaterial> material;
		Object *collider_obj = collision->get_collider();
		StaticBody3D *static_collider = Object::cast_to<StaticBody3D>(collider_obj);
		if (static_collider != nullptr) {
			material = static_collider->get_physics_material_override();
		} else {
			RigidBody3D *rigid_collider = Object::cast_to<RigidBody3D>(collider_obj);
			if (rigid_collider != nullptr) {
				material = rigid_collider->get_physics_material_override();
			}
		}

		if (material.is_valid()) {
			current_floor_friction = material->get_friction();
			current_floor_bounce = material->get_bounce();
		}
		break;
	}

	// Impact bounce: landing on a surface with bounce > 0 (e.g. the metal
	// PhysicsMaterial) reflects part of the fall speed back up immediately.
	if (is_on_floor() && !was_on_floor && current_floor_bounce > 0.0 && impact_velocity_y < -1.0) {
		Vector3 bounced_velocity = get_velocity();
		bounced_velocity.y = -impact_velocity_y * (real_t)current_floor_bounce;
		set_velocity(bounced_velocity);
	}

	// CharacterBody3D is kinematic: move_and_slide() never pushes the
	// RigidBody3D props it slides against, so we do it by hand. This must be
	// a *force* (apply_central_force), not an impulse: a force is cleared and
	// re-integrated by the physics server every single step, so leaning on a
	// prop for many ticks in a row accelerates it smoothly. An impulse is an
	// instant velocity kick — applying one every tick while still in contact
	// stacks a full kick 60 times a second, which sends the prop flying
	// (measured: an 18kg crate covered 42m in 1.5s of contact before this was
	// fixed). Force is naturally mass-independent — Godot divides it by each
	// body's own `mass` when integrating, so heavier props (see
	// game/main.tscn) accelerate less for the same push.
	Vector3 horizontal_velocity = Vector3(velocity.x, 0.0, velocity.z);
	real_t horizontal_speed = horizontal_velocity.length();
	double mass_factor = player_mass / 80.0;

	for (int i = 0; i < get_slide_collision_count(); i++) {
		Ref<KinematicCollision3D> collision = get_slide_collision(i);
		RigidBody3D *rigid_collider = Object::cast_to<RigidBody3D>(collision->get_collider());
		if (rigid_collider == nullptr) {
			continue;
		}

		Vector3 push_direction = -collision->get_normal();
		push_direction.y = 0.0;
		if (push_direction.length_squared() > 0.0) {
			rigid_collider->apply_central_force(push_direction.normalized() * (real_t)push_force * (real_t)mass_factor * MAX(horizontal_speed, (real_t)1.0));
		}
	}
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

void Player::set_push_force(double p_push_force) {
	push_force = p_push_force;
}

double Player::get_push_force() const {
	return push_force;
}
