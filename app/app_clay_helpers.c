/*
File:   app_clay_helpers.c
Author: Taylor Robbins
Date:   02\16\2025
Description: 
	** Holds functions that help us write Clay UI a bit more tersely
*/

#if BUILD_WITH_CLAY

Clay_ElementDeclaration ClayFullscreenContainer(const char* id, u16 topPadding)
{
	return (Clay_ElementDeclaration){
		.id = ToClayIdNt(id),
		.layout = {
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
			.padding = { .top = topPadding },
			.childGap = CLAY_DEF_PADDING,
		},
	};
}

Clay_ElementDeclaration ClayTopbar(const char* id, r32 height, Color32 color)
{
	return (Clay_ElementDeclaration){
		.id = ToClayIdNt(id),
		.layout = {
			.sizing = {
				.height = CLAY_SIZING_FIXED(height),
				.width = CLAY_SIZING_GROW(0),
			},
			.padding = { 0, 0, 0, 0 },
			.childGap = CLAY_DEF_PADDING,
			.childAlignment = { .y = CLAY_ALIGN_Y_CENTER },
		},
		.backgroundColor = ToClayColor(color),
	};
}

void ClayTextStr(Str8 text, u16 fontId, u16 fontSize, Color32 color)
{
	CLAY_TEXT(
		ToClayString(text),
		CLAY_TEXT_CONFIG({
			.fontId = fontId,
			.fontSize = fontSize,
			.textColor = ToClayColor(color),
		})
	);
}
void ClayText(const char* textNullTerm, u16 fontId, u16 fontSize, Color32 color)
{
	ClayTextStr(StrLit(textNullTerm), fontId, fontSize, color);
}


//Call Clay__CloseElement once if false, three times if true (i.e. twicfe inside the if statement and once after)
bool ClayTopBtn(const char* btnText, bool* isOpenPntr, Color32 backColor, Color32 textColor, r32 dropDownWidth)
{
	ScratchBegin(scratch);
	Color32 highlightColor = ColorLerpSimple(backColor, White, 0.3f);
	Str8 btnIdStr = PrintInArenaStr(scratch, "%s_TopBtn", btnText);
	Str8 menuIdStr = PrintInArenaStr(scratch, "%s_TopBtnMenu", btnText);
	Clay_ElementId btnId = ToClayId(btnIdStr);
	Clay_ElementId menuId = ToClayId(menuIdStr);
	bool isBtnHoveredOrMenuOpen = (Clay_PointerOver(btnId) || *isOpenPntr);
	Clay__OpenElement();
	Clay__ConfigureOpenElement((Clay_ElementDeclaration){
		.id = btnId,
		.layout = { .padding = { CLAY_DEF_PADDING*6, CLAY_DEF_PADDING*6, CLAY_DEF_PADDING*4, CLAY_DEF_PADDING*4 } },
		.backgroundColor = ToClayColor(isBtnHoveredOrMenuOpen ? highlightColor : backColor),
		.cornerRadius = CLAY_CORNER_RADIUS(5),
	});
	ClayText(btnText, app->clayFont, 18, textColor);
	bool isHovered = (Clay_PointerOver(btnId) || Clay_PointerOver(menuId));
	if (Clay_PointerOver(btnId) && IsMouseBtnPressed(&appIn->mouse, MouseBtn_Left)) { *isOpenPntr = !*isOpenPntr; }
	if (*isOpenPntr == true && !isHovered) { *isOpenPntr = false; }
	if (*isOpenPntr)
	{
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.id = menuId,
			.floating = {
				.attachTo = CLAY_ATTACH_TO_PARENT,
				.attachPoints = {
					.parent = CLAY_ATTACH_POINT_LEFT_BOTTOM,
				},
			},
			.layout = {
				.padding = { 0, 0, 0, 0 },
			}
		});
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.layout = {
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_FIXED(dropDownWidth),
				},
				.childGap = 2,
			},
			.backgroundColor = ToClayColor(MonokaiBack),
			.cornerRadius = CLAY_CORNER_RADIUS(8),
		});
	}
	ScratchEnd(scratch);
	return *isOpenPntr;
}

//Call Clay__CloseElement once after if statement
bool ClayBtn(const char* btnText, Color32 backColor, Color32 textColor)
{
	ScratchBegin(scratch);
	Color32 hoverColor = ColorLerpSimple(backColor, White, 0.3f);
	Color32 pressColor = ColorLerpSimple(backColor, White, 0.1f);
	Str8 btnIdStr = PrintInArenaStr(scratch, "%s_Btn", btnText);
	Clay_ElementId btnId = ToClayId(btnIdStr);
	bool isHovered = Clay_PointerOver(btnId);
	bool isPressed = (isHovered && IsMouseBtnDown(&appIn->mouse, MouseBtn_Left));
	Clay__OpenElement();
	Clay__ConfigureOpenElement((Clay_ElementDeclaration){
		.id = btnId,
		.layout = {
			.padding = CLAY_PADDING_ALL(CLAY_DEF_PADDING*4),
			.sizing = { .width = CLAY_SIZING_GROW(0), },
		},
		.backgroundColor = ToClayColor(isPressed ? pressColor : (isHovered ? hoverColor : backColor)),
		.cornerRadius = CLAY_CORNER_RADIUS(8),
	});
	ClayText(btnText, app->clayFont, 18, textColor);
	ScratchEnd(scratch);
	return (isHovered && IsMouseBtnPressed(&appIn->mouse, MouseBtn_Left));
}

bool ClaySlider(const char* id, r32* valueOut, r32 valueOutMin, r32 valueOutMax, r32 width, r32 height, Color32 backColor, Color32 barColor)
{
	bool valueChanged = false;
	ScratchBegin(scratch);
	r32 valuePercent = ClampR32(InverseLerpR32(valueOutMin, valueOutMax, *valueOut), 0, 1);
	Str8 sliderIdStr = PrintInArenaStr(scratch, "%s_Slider", id);
	Str8 barIdStr = PrintInArenaStr(scratch, "%s_SliderBar", id);
	Clay_ElementId sliderId = ToClayId(sliderIdStr);
	Clay_ElementId barId = ToClayId(barIdStr);
	bool isHovered = (Clay_PointerOver(sliderId) || Clay_PointerOver(barId));
	bool isMoving = (isHovered && IsMouseBtnDown(&appIn->mouse, MouseBtn_Left));
	CLAY({ .layout = { .padding = { CLAY_DEF_PADDING*2, CLAY_DEF_PADDING*2, CLAY_DEF_PADDING, CLAY_DEF_PADDING } } })
	{
		CLAY({ .id = sliderId,
			.layout = {
				.sizing = { .width = width, .height = height },
				.padding = { 1, 1, 1, 1 },
			},
			.backgroundColor = ToClayColor(backColor),
			.cornerRadius = CLAY_CORNER_RADIUS(8),
		})
		{
			CLAY({ .id = barId,
				.layout = {
					.sizing = { .width = (width-2) * valuePercent, .height = (height-2) },
				},
				.backgroundColor = ToClayColor(barColor),
				.cornerRadius = CLAY_CORNER_RADIUS(8),
			}){}
		}
	}
	if (isMoving)
	{
		Clay_BoundingBox sliderLayoutBounds = Clay_FindElementBoundsById(sliderId);
		if (sliderLayoutBounds.width > 0 && sliderLayoutBounds.height > 0)
		{
			valuePercent = ClampR32((appIn->mouse.position.X - sliderLayoutBounds.x) / sliderLayoutBounds.width, 0, 1);
			r32 newValue = LerpR32(valueOutMin, valueOutMax, valuePercent);
			if (*valueOut != newValue)
			{
				*valueOut = newValue;
				valueChanged = true;
			}
		}
	}
	ScratchEnd(scratch);
	return valueChanged;
}
bool ClayLabeledSlider(const char* labelStr, u16 labelFontId, u16 labelFontSize, Color32 labelColor, r32* valueOut, r32 valueOutMin, r32 valueOutMax, r32 width, r32 height, Color32 backColor, Color32 barColor)
{
	bool valueChanged = false;
	CLAY({ .layout = { .layoutDirection = CLAY_LEFT_TO_RIGHT, .childGap = 8 } })
	{
		ClayText(labelStr, labelFontId, labelFontSize, labelColor);
		valueChanged = ClaySlider(labelStr, valueOut, valueOutMin, valueOutMax, width, height, backColor, barColor);
	}
	return valueChanged;
}

#endif //BUILD_WITH_CLAY
