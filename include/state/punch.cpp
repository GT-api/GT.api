#include "database/items.hpp"
#include "database/peer.hpp"
#include "database/world.hpp"
#include "network/packet.hpp"
#include "punch.hpp"

void punch(ENetEvent event, state state) 
{
    try
    {
        if (not create_rt(event, 0, 120)) return;
        short block1D = state.punch[1] * 100 + state.punch[0]; // 2D (x, y) to 1D ((destY * y + destX)) formula
        block& b = worlds[_peer[event.peer]->recent_worlds.back()].blocks[block1D];
        if (state.id == 18) // @note punching a block
        {
            if (b.bg == 0 and b.fg == 0) return;
            if (items[b.fg].type == std::byte{ type::STRONG }) throw std::runtime_error("It's too strong to break.");
            if (items[b.fg].type == std::byte{ type::MAIN_DOOR }) throw std::runtime_error("(stand over and punch to use)");
            block_punched(event, state, block1D); // TODO- referance it?
            if (b.fg not_eq 0 and b.hits[0] >= items[b.fg].hits) b.fg = 0;
            else if (b.bg not_eq 0 and b.hits[1] >= items[b.bg].hits) b.bg = 0;
            else return;
            if (rand() % 7 == 0)
            {
                drop_visuals(event, 
                    {112, 1/* @todo get real gt gem amount for each item. */},
                    {
                        static_cast<float>(state.punch[0]) + (0.08f + (static_cast<float>(rand()) / RAND_MAX) * (0.6f - 0.08f)), 
                        static_cast<float>(state.punch[1]) + (0.08f + (static_cast<float>(rand()) / RAND_MAX) * (0.6f - 0.08f))
                    }
                );
            }
        }
        else if (items[state.id].cloth_type not_eq clothing::none) return;
        else if (state.id == 32) 
        {
            if (items[b.fg].type == std::byte{ type::DOOR }) 
            {
                gt_packet(*event.peer, false, {
                    "OnDialogRequest",
                    std::format("set_default_color|`o\n"
                    "add_label_with_icon|big|`wEdit {}``|left|{}|\n"
                    "add_text_input|door_name|Label|{}|100|\n"
                    "add_popup_name|DoorEdit|\n"
                    "add_text_input|door_target|Destination||24|\n"
                    "add_smalltext|Enter a Destination in this format: `2WORLDNAME:ID``|left|\n"
                    "add_smalltext|Leave `2WORLDNAME`` blank (:ID) to go to the door with `2ID`` in the `2Current World``.|left|\n"
                    "add_text_input|door_id|ID||11|\n"
                    "add_smalltext|Set a unique `2ID`` to target this door as a Destination from another!|left|\n"
                    "add_checkbox|checkbox_locked|Is open to public|1\n"
                    "embed_data|tilex|{}\n"
                    "embed_data|tiley|{}\n"
                    "end_dialog|door_edit|Cancel|OK|", items[b.fg].raw_name, b.fg, b.label, state.punch[0], state.punch[1]).c_str()
                });
            }
            else if (items[b.fg].type == std::byte{ type::SIGN }) 
            {
                gt_packet(*event.peer, false, {
                    "OnDialogRequest",
                    std::format("set_default_color|`o\n"
                    "add_popup_name|SignEdit|\n"
                    "add_label_with_icon|big|`wEdit {}``|left|{}|\n"
                    "add_textbox|What would you like to write on this sign?``|left|\n"
                    "add_text_input|sign_text||{}|128|\n"
                    "embed_data|tilex|{}\n"
                    "embed_data|tiley|{}\n"
                    "end_dialog|sign_edit|Cancel|OK|", items[b.fg].raw_name, b.fg, b.label, state.punch[0], state.punch[1]).c_str()
                });
            }
            else if (items[b.fg].type == std::byte{ type::ENTRANCE }) 
            {
                gt_packet(*event.peer, false, {
                    "OnDialogRequest",
                    std::format("set_default_color|`o\n"
                    "set_default_color|`o"
                    "add_label_with_icon|big|`wEdit {}``|left|{}|"
                    "add_checkbox|checkbox_public|Is open to public|1"
                    "embed_data|tilex|{}"
                    "embed_data|tiley|{}"
                    "end_dialog|gateway_edit|Cancel|OK|", items[b.fg].raw_name, b.fg, state.punch[0], state.punch[1]).c_str()
                });
            }
            return; // @note wrench passes state_visuals() else blocks glitchs to wrench visuals...
        }
        else // @note placing a block
        {
            if (items[state.id].type == std::byte{ type::LOCK }) 
            {
                // @note checks if world is owned by someone already.
                if (worlds[_peer[event.peer]->recent_worlds.back()].owner == 00)
                    worlds[_peer[event.peer]->recent_worlds.back()].owner = _peer[event.peer]->user_id;
                    // @todo update visuals...
                else throw std::runtime_error("Only one `$World Lock`` can be placed in a world, you'd have to remove the other one first.");
            }
            if (items[state.id].collision == collision::full)
            {
                // @visual O<-|
                if (
                    _peer[event.peer]->facing_left and
                    state.punch.front() == std::lround(state.pos.front() / 32) and 
                    state.punch.back() == std::lround(state.pos.back() / 32)
                ) return;

                // @visual OO<-|
                if (
                    _peer[event.peer]->facing_left and
                    state.punch.front() == std::lround(state.pos.front() / 32) + 1 and 
                    state.punch.back() == std::lround(state.pos.back() / 32) + 1
                ) return;

                // @visual |->OO
                if (
                    not _peer[event.peer]->facing_left and
                    state.punch.front() == std::lround(state.pos.front() / 32) + 1 and 
                    state.punch.back() == std::lround(state.pos.back() / 32) + 1
                ) return;

                // @visual |->O
                if (
                    not _peer[event.peer]->facing_left and
                    state.punch.front() == std::lround(state.pos.front() / 32) and 
                    state.punch.back() == std::lround(state.pos.back() / 32)
                ) return;
            }
            (items[state.id].type == std::byte{ type::BACKGROUND }) ? b.bg = state.id : b.fg = state.id; // @note this helps prevent foregrounds to act as backgrounds.
            _peer[event.peer]->emplace(slot{
                static_cast<short>(state.id),
                -1 // @note remove that item the peer just placed.
            });
        }
        state_visuals(event, std::move(state)); // finished.
    }
    catch (const std::exception& exc)
    {
        if (exc.what() and *exc.what()) 
            gt_packet(*event.peer, false, {
                "OnTalkBubble", 
                1u,
                exc.what()
            });
    }
}