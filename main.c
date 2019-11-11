#include <gb/gb.h>
#include <stdio.h>
#include "big_cactus.c"
#include "sam_BG_tiles.c"
#include "sam_bg0.c"

UINT8 tick = 0;

// These are relative to the size of the cactus
UINT8 floor_position = 149;
UINT8 ceiling_position = 40;
UINT8 left_wall_position = 8;
UINT8 right_wall_position = 143;

UINT8 player_x = right_wall_position;
UINT8 player_y = floor_position;

UBYTE player_pose = 0; // 0 is standing, 1 is left foot up, 2 is right foot up, 3 is jumping
UBYTE attack_pose = 0; // 0 is standing, 1 is mid, 2 is attacking, 3 is mid, 4 is standing holding B
UBYTE player_direction = 1; // 1 is right, 0 is left

// Player tiles: 0-15
UINT8 player_tiles_start = 0;
UINT8 player_tiles_end = 16;

// More CPU efficient
void p_delay(UINT8 loops) {
  UINT8 h;
  for (h = 0; h < loops; h++)
    wait_vbl_done(); // Waits until the screen is finished drawing
}

void set_up_big_cactus() {
  UINT8 i; // multipurpose counter variable

  set_sprite_data(0x00, 0x2f, big_cactus);
  for (i = 0x00; i < 0x2f; i++) {
    set_sprite_tile(i, i);
  }

  move_sprite(0x00, player_x, player_y - 24);
  move_sprite(0x01, player_x, player_y - 16);
  move_sprite(0x02, player_x + 8, player_y - 24);
  move_sprite(0x03, player_x + 8, player_y - 16);
  move_sprite(0x04, player_x, player_y - 8);
  move_sprite(0x05, player_x, player_y);
  move_sprite(0x06, player_x + 8, player_y - 8);
  move_sprite(0x07, player_x + 8, player_y);
  move_sprite(0x08, player_x + 16, player_y - 24);
  move_sprite(0x09, player_x + 16, player_y - 16);
  move_sprite(0x0a, player_x + 24, player_y - 24);
  move_sprite(0x0b, player_x + 24, player_y - 16);
  move_sprite(0x0c, player_x + 16, player_y - 8);
  move_sprite(0x0d, player_x + 16, player_y);
  move_sprite(0x0e, player_x + 24, player_y - 8);
  move_sprite(0x0f, player_x + 24, player_y);
}

void set_player_pose(UBYTE pose) {
  switch (pose) {
    case 0:
      player_pose = 0;
      set_sprite_tile(0x05, 0x05); // Left leg
      set_sprite_tile(0x0d, 0x0d); // Right leg
      break;
    case 1:
      player_pose = 1;
      set_sprite_tile(0x05, 0x15);
      set_sprite_tile(0x0d, 0x0d);
      break;
    case 2:
      player_pose = 2;
      set_sprite_tile(0x05, 0x05);
      set_sprite_tile(0x0d, 0x2d);
      break;
    case 3:
      player_pose = 3;
      set_sprite_tile(0x05, 0x15);
      set_sprite_tile(0x0d, 0x2d);
      break;
  }
}

void move_cactus() {
  UINT8 i;

  if (joypad() & J_LEFT && player_x - 1 >= left_wall_position) {
    for (i = 0; i < player_tiles_end; i++) {
      scroll_sprite(i, -1, 0);
    }
    player_x--;
    if (!(tick % 6) && player_pose != 3) { // Prevents leg motion while jumping
      set_player_pose((player_pose % 2) + 1);
    }
    player_direction = 0;
    return;
  }

  if (joypad() & J_RIGHT && player_x + 1 <= right_wall_position) {
    for (i = 0; i < player_tiles_end; i++) {
      scroll_sprite(i, 1, 0);
    }
    player_x++;
    if (!(tick % 6) && player_pose != 3) { // Prevents leg motion while jumping
      set_player_pose((player_pose % 2) + 1);
    }
    player_direction = 1;
    return;
  }

  set_player_pose(0);
}

void cactus_attack() {
  UINT8 i; 

  if (!attack_pose) {
    attack_pose = 1;
    for (i = 0x00; i < 0x10; i++) {
      if (i != 0x05 && i != 0x0d) {
        set_sprite_tile(i, i + 16);
      }
    }

    return;
  }

  if (tick % 3) // Slow down the attack
    return;
  
  attack_pose++;

  if (attack_pose == 2) {
    for (i = 0x00; i < 0x10; i++) {
      if (i != 0x05 && i != 0x0d) { // Don't move the feet, they need to walk
        set_sprite_tile(i, i + 32);
      }
    }

    return;
  }

  if (attack_pose == 3) {
    for (i = 0x00; i < 0x10; i++) {
      if (i != 0x05 && i != 0x0d) {
        set_sprite_tile(i, i + 16);
      }
    }

    return;
  }

  if (attack_pose == 4) {
    for (i = 0x00; i < 0x10; i++) {
      if (i != 0x05 && i != 0x0d) {
        set_sprite_tile(i, i);
      }
    }

    return;
  }
}

void main() {
  SPRITES_8x8;
  set_up_big_cactus();

  set_bkg_data(0, 114, SamBackground0Tiles);
  set_bkg_tiles(0, 0, 20, 18, SamBG0);

  SHOW_BKG;
  SHOW_SPRITES;
  DISPLAY_ON;

  while (1) {
    tick++;

    if (!attack_pose && joypad() & J_B) {
      cactus_attack();
    }
    else if (attack_pose && attack_pose != 4) {
      cactus_attack();
    }
    else if (attack_pose == 4 && !(joypad() & J_B)) {
      attack_pose = 0;
    }

    move_cactus();

    p_delay(1);
  }
}