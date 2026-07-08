#include "IndicatorEntryWidget.h"

void UIndicatorEntryWidget::UpdateVisual(const FLMSIndicatorScreenData& NewData)
{
	IndicatorData = NewData;
	OnIndicatorDataUpdated();
}
