#ifndef FPS_PLAYER_H
#define FPS_PLAYER_H

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>

namespace godot {

class Player : public CharacterBody3D {
	GDCLASS(Player, CharacterBody3D)

private:
	double speed = 6.0;
	double jump_velocity = 4.5;
	double gravity = 9.8;
	double mouse_sensitivity = 0.003;

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
};

} // namespace godot

#endif // FPS_PLAYER_H
