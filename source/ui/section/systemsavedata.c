#include <malloc.h>
#include <stdio.h>

#include <3ds.h>

#include "action/action.h"
#include "section.h"
#include "../error.h"
#include "../../screen.h"

#define SYSTEMSAVEDATA_MAX 512

typedef struct {
    list_item items[SYSTEMSAVEDATA_MAX];
    u32 count;
    Handle cancelEvent;
    bool populated;
} systemsavedata_data;

#define SYSTEMSAVEDATA_ACTION_COUNT 2

static u32 systemsavedata_action_count = SYSTEMSAVEDATA_ACTION_COUNT;
static list_item systemsavedata_action_items[SYSTEMSAVEDATA_ACTION_COUNT] = {
        {"Browse Save Data", COLOR_TEXT, action_browse_system_save_data},
        {"Delete Save Data", COLOR_TEXT, action_delete_system_save_data},
};

typedef struct {
    system_save_data_info* info;
    bool* populated;
} systemsavedata_action_data;

static void systemsavedata_action_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    ui_draw_system_save_data_info(view, ((systemsavedata_action_data*) data)->info, x1, y1, x2, y2);
}

static void systemsavedata_action_update(ui_view* view, void* data, list_item** items, u32** itemCount, list_item* selected, bool selectedTouched) {
    systemsavedata_action_data* actionData = (systemsavedata_action_data*) data;

    if(hidKeysDown() & KEY_B) {
        ui_pop();
        list_destroy(view);

        free(data);

        return;
    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        void(*action)(system_save_data_info*, bool*) = (void(*)(system_save_data_info*, bool*)) selected->data;

        ui_pop();
        list_destroy(view);

        action(actionData->info, actionData->populated);

        free(data);

        return;
    }

    if(*itemCount != &systemsavedata_action_count || *items != systemsavedata_action_items) {
        *itemCount = &systemsavedata_action_count;
        *items = systemsavedata_action_items;
    }
}

static void systemsavedata_action_open(system_save_data_info* info, bool* populated) {
    systemsavedata_action_data* data = (systemsavedata_action_data*) calloc(1, sizeof(systemsavedata_action_data));
    if(data == NULL) {
        error_display(NULL, NULL, NULL, "Failed to allocate system save data action data.");

        return;
    }

    data->info = info;
    data->populated = populated;

    list_display("System Save Data Action", "A: Select, B: Return", data, systemsavedata_action_update, systemsavedata_action_draw_top);
}

static void systemsavedata_draw_top(ui_view* view, void* data, float x1, float y1, float x2, float y2, list_item* selected) {
    if(selected != NULL && selected->data != NULL) {
        ui_draw_system_save_data_info(view, selected->data, x1, y1, x2, y2);
    }
}

static void systemsavedata_update(ui_view* view, void* data, list_item** items, u32** itemCount, list_item* selected, bool selectedTouched) {
    systemsavedata_data* listData = (systemsavedata_data*) data;

    if(hidKeysDown() & KEY_B) {
        if(listData->cancelEvent != 0) {
            svcSignalEvent(listData->cancelEvent);
            while(svcWaitSynchronization(listData->cancelEvent, 0) == 0) {
                svcSleepThread(1000000);
            }

            listData->cancelEvent = 0;
        }

        ui_pop();
        list_destroy(view);

        task_clear_system_save_data(listData->items, &listData->count);
        free(listData);
        return;
    }

    if(!listData->populated || (hidKeysDown() & KEY_X)) {
        if(listData->cancelEvent != 0) {
            svcSignalEvent(listData->cancelEvent);
            while(svcWaitSynchronization(listData->cancelEvent, 0) == 0) {
                svcSleepThread(1000000);
            }

            listData->cancelEvent = 0;
        }

        listData->cancelEvent = task_populate_system_save_data(listData->items, &listData->count, SYSTEMSAVEDATA_MAX);
        listData->populated = true;
    }

    if(selected != NULL && selected->data != NULL && (selectedTouched || (hidKeysDown() & KEY_A))) {
        systemsavedata_action_open((system_save_data_info*) selected->data, &listData->populated);
        return;
    }

    if(*itemCount != &listData->count || *items != listData->items) {
        *itemCount = &listData->count;
        *items = listData->items;
    }
}

void systemsavedata_open() {
    systemsavedata_data* data = (systemsavedata_data*) calloc(1, sizeof(systemsavedata_data));
    if(data == NULL) {
        error_display(NULL, NULL, NULL, "Failed to allocate system save data data.");

        return;
    }

    list_display("System Save Data", "A: Select, B: Return, X: Refresh", data, systemsavedata_update, systemsavedata_draw_top);
}