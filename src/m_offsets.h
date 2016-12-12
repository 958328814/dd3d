#ifndef M_OFFSETS_H_INCLUDED
#define M_OFFSETS_H_INCLUDED

#include "database.h"
#include "utils.h"

struct OffsetDataInfo {
	uint8_t* data;
	uint32_t size;
};

typedef std::map<uint32_t, OffsetDataInfo> OffsetDataMapType;

void m_offsets_init(void);
void m_offsets_cleanup(void);
const OffsetDataInfo* m_offsets_get_data(uint32_t version);

#endif
