#include "dd3d.h"
#include <sys/types.h>
#include <dirent.h>
#include "syslog.h"
#include "config.h"
#include "utils.h"

#include "m_offsets.h"

OffsetDataMapType OffsetDataMap;

void m_offsets_init(void) {
	const char* offset_file_dir;
	if (CONFIG_TRUE == config_lookup_string(config_get(), "dd3d.offset_file_dir", &offset_file_dir)) {
		DIR* dir;
		dirent* ent;
		char buffer[5] = {0};
		char path[1024];
		if (NULL != (dir = opendir(offset_file_dir))) {
			bool ok = false;
			while (NULL != (ent = readdir(dir))) {
				if (strlen(ent->d_name) == 8) {
					sprintf(path, "%s%s", offset_file_dir, ent->d_name);
					strncpy(buffer, ent->d_name, 4);
					uint32_t ver = static_cast<uint32_t>(atoi(buffer));
					FILE* fp = fopen(path, "rb");
					if (fp) {
						if (fseek(fp, 0, SEEK_END) == 0) {
							uint32_t file_size = ftell(fp);
							rewind(fp);
							if (file_size) {
								OffsetDataInfo& info = OffsetDataMap[ver];
								info.size = file_size;
								info.data = new uint8_t[file_size];
								uint32_t read_size = fread(info.data, 1, file_size, fp);
								if (file_size == read_size) {
									ok = true;
								} else {
									syslog(LOG_ERR, "[ERROR]Fread failed want %u get %u: %s\n", file_size, read_size, path);
								}
							}
						} else {
							syslog(LOG_ERR, "[ERROR]Fseek field: %s\n", path);				
						}
						fclose(fp);
					} else
						syslog(LOG_ERR, "[ERROR]Open offset file field: %s\n", path);
				}
			}
			closedir(dir);
			if (!ok)
				throw "Init offsets failed.\n";
		} else
			syslog(LOG_ERR, "[ERROR]Faild to open offsets dir.\n");
	} else
		throw "Offset file dir config is missing.\n";
}

const OffsetDataInfo* m_offsets_get_data(uint32_t version) {
	OffsetDataMapType::iterator iter = OffsetDataMap.find(version);
	return iter == OffsetDataMap.end() ? NULL : &(iter->second);
}

void m_offsets_cleanup(void) {
	OffsetDataMapType::iterator iter;
	for (iter = OffsetDataMap.begin(); iter != OffsetDataMap.end(); ++iter) {
		delete [] iter->second.data;
	}
}
