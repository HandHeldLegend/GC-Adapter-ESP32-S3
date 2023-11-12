#include "xinput.h"

void xinput_report(gc_poll_response_s *poll_reponse, gc_origin_data_s *origin_data)
{
    static xid_input_s xid_input = {0};

    xid_input.report_id = 0x00;
    xid_input.report_size = 20;

    if (cmd_phase != CMD_PHASE_POLL)
    {
        xid_input.buttons_1 = 0x00;
        xid_input.buttons_2 = 0x00;
        xid_input.stick_left_x = 0;
        xid_input.stick_left_y = 0;
        xid_input.stick_right_x = 0;
        xid_input.stick_right_y = 0;
        xid_input.analog_trigger_l = 0;
        xid_input.analog_trigger_r = 0;
    }
    else
    {

        uint32_t regread = REG_READ(GPIO_IN_REG) & PIN_MASK_GCP;
        xid_input.button_guide = !util_getbit(regread, NEXT_BUTTON);
        xid_input.button_back =  !util_getbit(regread, PREV_BUTTON);

        xid_input.button_a = gc_poll_response.button_a;
        xid_input.button_b = gc_poll_response.button_b;

        xid_input.dpad_left     = gc_poll_response.dpad_left;
        xid_input.dpad_right    = gc_poll_response.dpad_right;
        xid_input.dpad_down     = gc_poll_response.dpad_down;
        xid_input.dpad_up       = gc_poll_response.dpad_up;

        //Defaults
        xid_input.button_y       = gc_poll_response.button_y;
        xid_input.button_x       = gc_poll_response.button_x;
        xid_input.bumper_r       = gc_poll_response.button_z;
        xid_input.bumper_l       = 0;
        xid_input.button_menu    = 0;

        if (adapter_settings.zjump == 1)
        {
            xid_input.button_x       = gc_poll_response.button_z;
            xid_input.bumper_r       = gc_poll_response.button_x;
        }
        else if (adapter_settings.zjump == 2)
        {
            xid_input.button_y       = gc_poll_response.button_z;
            xid_input.bumper_r       = gc_poll_response.button_y;
        }

        if (gc_poll_response.button_start && gc_poll_response.button_z)
        {
            xid_input.bumper_l = 1;
            xid_input.bumper_r = 0;
            xid_input.button_x = 0;
        }
        else if (gc_poll_response.button_start)
        {
            xid_input.button_menu = 1;
        }

        int adj_x   = gc_origin_adjust(gc_poll_response.stick_x,  gc_origin_data.stick_x,     false);
        int adj_y   = gc_origin_adjust(gc_poll_response.stick_y,  gc_origin_data.stick_y,     true);
        int adj_cx  = gc_origin_adjust(gc_poll_response.cstick_x, gc_origin_data.cstick_x,    false);
        int adj_cy  = gc_origin_adjust(gc_poll_response.cstick_y, gc_origin_data.cstick_y,    true);

        int adj_tl  = gc_origin_adjust(gc_poll_response.trigger_l, gc_origin_data.trigger_l,  false);
        int adj_tr  = gc_origin_adjust(gc_poll_response.trigger_r, gc_origin_data.trigger_r,  false);

        switch( adapter_settings.trigger_mode_l)
        {
            default:
            case TRIG_MODE_OFF:
                xid_input.analog_trigger_l = scale_trigger(adj_tl);
                break;

            case TRIG_MODE_A2D:
                xid_input.analog_trigger_l   = gc_poll_response.button_l * 255;
                break;
        }

        switch( adapter_settings.trigger_mode_r)
        {
            default:
            case TRIG_MODE_OFF:
                xid_input.analog_trigger_r = scale_trigger(adj_tr);
                break;

            case TRIG_MODE_A2D:
                xid_input.analog_trigger_r   = gc_poll_response.button_r * 255;
                break;
        }

        xid_input.stick_left_x   = sign_axis(adj_x);
        xid_input.stick_left_y   = sign_axis(adj_y)*-1;
        xid_input.stick_right_x  = sign_axis(adj_cx);
        xid_input.stick_right_y  = sign_axis(adj_cy)*-1;
    }

    tud_xinput_report(&xid_input, XID_REPORT_LEN);

}
