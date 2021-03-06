#include <3ds.h>

#include "action.h"
#include "../section.h"

void action_browse_boss_ext_save_data(ext_save_data_info* info, bool* populated) {
    u32 path[3] = {info->mediaType, (u32) (info->extSaveDataId & 0xFFFFFFFF), (u32) ((info->extSaveDataId >> 32) & 0xFFFFFFFF)};
    FS_Archive archive = {ARCHIVE_BOSS_EXTDATA, {PATH_BINARY, 12, path}};
    files_open(archive);
}