

void growid(ENetEvent& event, const std::string& header, std::string error = "")
{
    std::this_thread::sleep_for(500ms); /* unknown reason, due to slow reponse I reduce to 500ms, it surprisingly worked!- real GT has to up there game xD  */
    gt_packet(*event.peer, 0, false, "OnDialogRequest", 
std::format(R"(text_scaling_string|Dirttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttttt|
set_default_color|`o
add_label_with_icon|big|`wGet a GrowID``|left|206|
add_spacer|small|
add_textbox|{0}
add_textbox|By choosing a `wGrowID``, you can use a name and password to logon from any device. Your `wname`` will be shown to other players!|left|
add_spacer|small|
add_text_input|logon|Name||18|
add_textbox|Your `wpassword`` must contain `w8 to 18 characters, 1 letter, 1 number`` and `w1 special character: @#!$^&*.,``|left|
add_text_input_password|password|Password||18|
add_text_input_password|password_verify|Password Verify||18|
add_textbox|Your `wemail`` will only be used for account verification and support. If you enter a fake email, you can't verify your account, recover or change your password.|left|
add_text_input|email|Email||64|
add_textbox|We will never ask you for your password or email, never share it with anyone!|left|
end_dialog|growid_apply|Cancel|Get My GrowID!|)", (error.empty() ? "" : error + "|left|\nadd_spacer|small|")).c_str());
}