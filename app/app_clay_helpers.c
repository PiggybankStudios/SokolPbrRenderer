/*
File:   app_clay_helpers.c
Author: Taylor Robbins
Date:   02\16\2025
Description: 
	** Holds functions that help us write Clay UI a bit more tersely
*/

Clay_ElementDeclaration ClayFullscreenContainer(const char* id)
{
	return (Clay_ElementDeclaration){
		.id = ToClayIdNt(id),
		.layout = {
			.layoutDirection = CLAY_TOP_TO_BOTTOM,
			.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
			.padding = CLAY_PADDING_ALL(CLAY_DEF_PADDING),
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
		.cornerRadius = CLAY_CORNER_RADIUS(8)
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
bool ClayTopBtn(const char* btnText, Color32 backColor, Color32 textColor)
{
	ScratchBegin(scratch);
	Str8 btnIdStr = PrintInArenaStr(scratch, "%s_TopBtn", btnText);
	Str8 menuIdStr = PrintInArenaStr(scratch, "%s_TopBtnMenu", btnText);
	Clay_ElementId btnId = ToClayId(btnIdStr);
	Clay_ElementId menuId = ToClayId(menuIdStr);
	Clay__OpenElement();
	Clay__ConfigureOpenElement((Clay_ElementDeclaration){
		.id = btnId,
		.layout = { .padding = { CLAY_DEF_PADDING, CLAY_DEF_PADDING, 8, 8 } },
		.backgroundColor = ToClayColor(backColor),
		.cornerRadius = CLAY_CORNER_RADIUS(5),
	});
	ClayText(btnText, app->clayFont, 18, textColor);
	bool isOpen = (Clay_PointerOver(btnId) || Clay_PointerOver(menuId));
	if (isOpen)
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
				.padding = { 0, 0, CLAY_DEF_PADDING/2, CLAY_DEF_PADDING/2 },
			}
		});
		
		Clay__OpenElement();
		Clay__ConfigureOpenElement((Clay_ElementDeclaration){
			.layout = {
				.layoutDirection = CLAY_TOP_TO_BOTTOM,
				.sizing = {
					.width = CLAY_SIZING_FIXED(200),
				},
			},
			.backgroundColor = ToClayColor(MonokaiBack),
			.cornerRadius = CLAY_CORNER_RADIUS(8)
		});
	}
	ScratchEnd(scratch);
	return isOpen;
}

//Call Clay__CloseElement once after if statement
bool ClayBtn(const char* btnText, Color32 backColor, Color32 textColor)
{
	ScratchBegin(scratch);
	Str8 btnIdStr = PrintInArenaStr(scratch, "%s_Btn", btnText);
	Clay_ElementId btnId = ToClayId(btnIdStr);
	Clay__OpenElement();
	Clay__ConfigureOpenElement((Clay_ElementDeclaration){
		.id = btnId,
		.layout = { .padding = CLAY_PADDING_ALL(CLAY_DEF_PADDING) },
		.backgroundColor = ToClayColor(backColor),
	});
	ClayText(btnText, app->clayFont, 18, textColor);
	ScratchEnd(scratch);
	return (Clay_PointerOver(btnId) && IsMouseBtnPressed(&appIn->mouse, MouseBtn_Left));
}
