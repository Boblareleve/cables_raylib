#include "json.h"
#include "main.h"

/* 
{
    "ui": {
        "menu": {
            "background shade": [0, 10, 30, 220]
        }
        
    },
    "keybind": {
    },
    "gameplay": {
    }
}


*/



bool update_options_Json(Window *win, Option *option, const Json json)
{
    if (json.root.id != Json_object) 
    {
        option->error_reading_file = "";
        return false;
    }


    Json_member *op_ui = Json_Search_member(json.root.object, Strv_cstr("ui"));
    if (op_ui)
    {
        assert(Str_equal(op_ui->name, Strv_cstr("ui")));
        if (op_ui->value.id != Json_object) return false;

    }

    return true;

}


bool update_options(Window *win, Option *option)
{
    Json_free(&option->option_json);
    Json json = Json_parse(win->option.path_option_file);
    if (!json.error.id)
    {
        option->error_reading_file = Json_get_error_string(json.error.id);
        return false;
    }

    option->option_json = json;
    
    return update_options_Json(win, option, json);
}
