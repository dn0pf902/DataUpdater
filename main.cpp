#include "DataUpdater.h"

IMod* BMLEntry(IBML* bml) {
	return new DataUpdater(bml);
}

void DataUpdater::OnStartLevel() {
	m_bml->SendIngameMessage("test");
	m_bml->SendIngameMessage("test");
}