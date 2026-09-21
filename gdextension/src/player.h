#ifndef FPS_PLAYER_H
#define FPS_PLAYER_H

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/kinematic_collision3d.hpp>
#include <godot_cpp/classes/physics_material.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>
#include <godot_cpp/classes/static_body3d.hpp>

namespace godot {

class Player : public CharacterBody3D {
	GDCLASS(Player, CharacterBody3D)

private:
	double speed = 6.0;
	double jump_velocity = 4.5;
	double gravity = 9.8;
	double mouse_sensitivity = 0.003;
	double push_force = 40.0;

	// Metadata read once in _ready(); see game/main.tscn's metadata/mass on Player.
	double player_mass = 80.0;

	// Friction/bounce of whatever the player is standing on right now, sampled
	// from the floor collider's PhysicsMaterial after each move_and_slide().
	// Used on the *next* physics tick, one frame behind, which is imperceptible.
	double current_floor_friction = 1.0;
	double current_floor_bounce = 0.0;

	Node3D *camera_pivot = nullptr;

protected:
	static void _bind_methods();

public:
	Player();
	~Player();

	void _ready() override;
	void _physics_process(double p_delta) override;
	void _unhandled_input(const Ref<InputEvent> &p_event) override;

	void set_speed(double p_speed);
	double get_speed() const;

	void set_jump_velocity(double p_jump_velocity);
	double get_jump_velocity() const;

	void set_mouse_sensitivity(double p_sensitivity);
	double get_mouse_sensitivity() const;

	void set_push_force(double p_push_force);
	double get_push_force() const;
};

} // namespace godot

#endif // FPS_PLAYER_H
