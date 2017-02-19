#include "config.h"
#include "GUIDeckEdit.h"
#include "data_manager.h"
#include "deck_manager.h"
#include "image_manager.h"
#include "game.h"
#include "duelclient.h"
#include <algorithm>

namespace ygo {

bool GUIDeckEdit::OnEvent(const irr::SEvent& event) {
	switch(event.EventType) {
	case irr::EET_GUI_EVENT: {
		s32 id = event.GUIEvent.Caller->getID();
		switch(event.GUIEvent.EventType) {
		case irr::gui::EGET_BUTTON_CLICKED: {
			switch(id) {
			case BUTTON_CLEAR_DECK: {
				deckManager.current_deck.main.clear();
				deckManager.current_deck.extra.clear();
				deckManager.current_deck.side.clear();
				break;
			}
			case BUTTON_CLEAR_FILTER:{
				ClearFilter();
				break;
			}
			case BUTTON_SORT_DECK: {
				std::sort(deckManager.current_deck.main.begin(), deckManager.current_deck.main.end(), ClientCard::deck_sort_lv);
				std::sort(deckManager.current_deck.extra.begin(), deckManager.current_deck.extra.end(), ClientCard::deck_sort_lv);
				std::sort(deckManager.current_deck.side.begin(), deckManager.current_deck.side.end(), ClientCard::deck_sort_lv);
				break;
			}
			case BUTTON_SHUFFLE_DECK: {
				std::random_shuffle(deckManager.current_deck.main.begin(), deckManager.current_deck.main.end());
				break;
			}
			case BUTTON_SAVE_DECK: {
				if(deckManager.SaveDeck(deckManager.current_deck, _ComboBox[COMBOBOX_DBDECKS]->getItem(_ComboBox[COMBOBOX_DBDECKS]->getSelected()))) {
					mainGame->stACMessage->setText(dataManager.GetSysString(1335));
					mainGame->PopupElement(mainGame->wACMessage, 20);
				}
				break;
			}
			case BUTTON_SAVE_DECK_AS: {
				const wchar_t* dname = _editBox[EDITBOX_DECKNAME]->getText();
				if(*dname == 0)
					break;
				int sel = -1;
				for(size_t i = 0; i < _ComboBox[COMBOBOX_DBDECKS]->getItemCount(); ++i) {
					if(!wcscmp(dname, _ComboBox[COMBOBOX_DBDECKS]->getItem(i))) {
						sel = i;
						break;
					}
				}
				if(sel >= 0)
					_ComboBox[COMBOBOX_DBDECKS]->setSelected(sel);
				else {
					_ComboBox[COMBOBOX_DBDECKS]->addItem(dname);
					_ComboBox[COMBOBOX_DBDECKS]->setSelected(_ComboBox[COMBOBOX_DBDECKS]->getItemCount() - 1);
				}
				if(deckManager.SaveDeck(deckManager.current_deck, dname)) {
					mainGame->stACMessage->setText(dataManager.GetSysString(1335));
					mainGame->PopupElement(mainGame->wACMessage, 20);
				}
				break;
			}
			case BUTTON_DBEXIT: {
				Hide();
				mainGame->wInfoTab.Hide();
				if(exit_on_return)
					mainGame->device->closeDevice();
				mainGame->wMenu.Show();
				break;
			}
			case BUTTON_EFFECT_FILTER: {
				mainGame->PopupElement(wCategories);
				break;
			}
			case BUTTON_START_FILTER: {
				FilterStart();
				FilterCards(true);
				break;
			}
			case BUTTON_CATEGORY_OK: {
				filter_effect = 0;
				long long filter = 0x1;
				for(int i = 0; i < 32; ++i, filter <<= 1)
					if(chkCategory[i]->isChecked())
						filter_effect |= filter;
				mainGame->HideElement(wCategories);
				break;
			}
			case BUTTON_SIDE_OK: {
				if(deckManager.current_deck.main.size() != pre_mainc || deckManager.current_deck.extra.size() != pre_extrac
				        || deckManager.current_deck.side.size() != pre_sidec) {
					mainGame->env->addMessageBox(L"", dataManager.GetSysString(1410));
					break;
				}
				char deckbuf[1024];
				char* pdeck = deckbuf;
				BufferIO::WriteInt32(pdeck, deckManager.current_deck.main.size() + deckManager.current_deck.extra.size());
				BufferIO::WriteInt32(pdeck, deckManager.current_deck.side.size());
				for(size_t i = 0; i < deckManager.current_deck.main.size(); ++i)
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.main[i]->first);
				for(size_t i = 0; i < deckManager.current_deck.extra.size(); ++i)
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.extra[i]->first);
				for(size_t i = 0; i < deckManager.current_deck.side.size(); ++i)
					BufferIO::WriteInt32(pdeck, deckManager.current_deck.side[i]->first);
				DuelClient::SendBufferToServer(CTOS_UPDATE_DECK, deckbuf, pdeck - deckbuf);
				break;
			}
			case BUTTON_DELETE_DECK: {

				mainGame->SetStaticText(mainGame->stQMessage, 310, mainGame->textFont, (wchar_t*)dataManager.GetSysString(2025));
				mainGame->PopupElement(mainGame->wQuery);
				break;
			}
			case BUTTON_YES: {
				mainGame->HideElement(mainGame->wQuery);				
				std::wstring filestr(L"./deck/");
				filestr += _ComboBox[COMBOBOX_DBDECKS]->getItem(_ComboBox[COMBOBOX_DBDECKS]->getSelected());
				filestr += L".ydk";
				char* path = new char[256];
				std::wcstombs(path,filestr.c_str(),256);
				if(remove(path) == 0){
					_ComboBox[COMBOBOX_DBDECKS]->removeItem(_ComboBox[COMBOBOX_DBDECKS]->getSelected());
					deckManager.current_deck.main.clear();
					deckManager.current_deck.extra.clear();
					deckManager.current_deck.side.clear();
				}
				break;
			}
			case BUTTON_NO: {
				mainGame->HideElement(mainGame->wQuery);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_ENTER: {
			switch(id) {
			case EDITBOX_KEYWORD: {
				irr::SEvent me;
				me.EventType = irr::EET_GUI_EVENT;
				me.GUIEvent.EventType = irr::gui::EGET_BUTTON_CLICKED;
				me.GUIEvent.Caller = _buttons[BUTTON_START_FILTER];
				me.GUIEvent.Element = _buttons[BUTTON_START_FILTER];
				mainGame->device->postEventFromUser(me);
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_EDITBOX_CHANGED: {
			switch(id) {
			case EDITBOX_KEYWORD: {
				stringw filter = _editBox[EDITBOX_KEYWORD]->getText();
				if(filter.size() > 2){
					FilterStart();
					FilterCards(false);
				}
				break;
			}
			}
			break;
		}
		case irr::gui::EGET_COMBO_BOX_CHANGED: {
			switch(id) {
			case COMBOBOX_DBLFLIST: {
				filterList = deckManager._lfList[_ComboBox[COMBOBOX_DBLFLIST]->getSelected()].content;
				break;
			}
			case COMBOBOX_DBDECKS: {
				deckManager.LoadDeck(_ComboBox[COMBOBOX_DBDECKS]->getItem(_ComboBox[COMBOBOX_DBDECKS]->getSelected()));
				break;
			}
			case COMBOBOX_MAINTYPE: {
				_ComboBox[COMBOBOX_MAINTYPE2]->setSelected(0);
				_ComboBox[COMBOBOX_ATTRIBUTE]->setSelected(0);
				_ComboBox[COMBOBOX_RACE]->setSelected(0);
				_editBox[EDITBOX_ATTACK]->setText(L"");
				_editBox[EDITBOX_DEFENCE]->setText(L"");
				_editBox[EDITBOX_LEVEL]->setText(L"");
				_editBox[EDITBOX_SCALE]->setText(L"");
				switch(_ComboBox[COMBOBOX_MAINTYPE]->getSelected()) {
				case 0: {
					_ComboBox[COMBOBOX_MAINTYPE2]->setEnabled(false);
					_ComboBox[COMBOBOX_RACE]->setEnabled(false);
					_ComboBox[COMBOBOX_ATTRIBUTE]->setEnabled(false);
					_editBox[EDITBOX_ATTACK]->setEnabled(false);
					_editBox[EDITBOX_DEFENCE]->setEnabled(false);
					_editBox[EDITBOX_LEVEL]->setEnabled(false);
					_editBox[EDITBOX_SCALE]->setEnabled(false);
					break;
				}
				case 1: {
					_ComboBox[COMBOBOX_MAINTYPE2]->setEnabled(true);
					_ComboBox[COMBOBOX_RACE]->setEnabled(true);
					_ComboBox[COMBOBOX_ATTRIBUTE]->setEnabled(true);
					_editBox[EDITBOX_ATTACK]->setEnabled(true);
					_editBox[EDITBOX_DEFENCE]->setEnabled(true);
					_editBox[EDITBOX_LEVEL]->setEnabled(true);
					_editBox[EDITBOX_SCALE]->setEnabled(true);
					_ComboBox[COMBOBOX_MAINTYPE2]->clear();
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1080), 0);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1054), TYPE_MONSTER + TYPE_NORMAL);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1055), TYPE_MONSTER + TYPE_EFFECT);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1056), TYPE_MONSTER + TYPE_FUSION);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1057), TYPE_MONSTER + TYPE_RITUAL);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1063), TYPE_MONSTER + TYPE_SYNCHRO);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1073), TYPE_MONSTER + TYPE_XYZ);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1062), TYPE_MONSTER + TYPE_TUNER);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1061), TYPE_MONSTER + TYPE_DUAL);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1060), TYPE_MONSTER + TYPE_UNION);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1059), TYPE_MONSTER + TYPE_SPIRIT);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1071), TYPE_MONSTER + TYPE_FLIP);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1072), TYPE_MONSTER + TYPE_TOON);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1074), TYPE_MONSTER + TYPE_PENDULUM);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1075), TYPE_MONSTER + TYPE_SPSUMMON);
					break;
				}
				case 2: {
					_ComboBox[COMBOBOX_MAINTYPE2]->setEnabled(true);
					_ComboBox[COMBOBOX_RACE]->setEnabled(false);
					_ComboBox[COMBOBOX_ATTRIBUTE]->setEnabled(false);
					_editBox[EDITBOX_ATTACK]->setEnabled(false);
					_editBox[EDITBOX_DEFENCE]->setEnabled(false);
					_editBox[EDITBOX_LEVEL]->setEnabled(false);
					_editBox[EDITBOX_SCALE]->setEnabled(false);
					_ComboBox[COMBOBOX_MAINTYPE2]->clear();
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1080), 0);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1054), TYPE_SPELL);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1066), TYPE_SPELL + TYPE_QUICKPLAY);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1067), TYPE_SPELL + TYPE_CONTINUOUS);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1057), TYPE_SPELL + TYPE_RITUAL);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1068), TYPE_SPELL + TYPE_EQUIP);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1069), TYPE_SPELL + TYPE_FIELD);
					break;
				}
				case 3: {
					_ComboBox[COMBOBOX_MAINTYPE2]->setEnabled(true);
					_ComboBox[COMBOBOX_RACE]->setEnabled(false);
					_ComboBox[COMBOBOX_ATTRIBUTE]->setEnabled(false);
					_editBox[EDITBOX_ATTACK]->setEnabled(false);
					_editBox[EDITBOX_DEFENCE]->setEnabled(false);
					_editBox[EDITBOX_LEVEL]->setEnabled(false);
					_editBox[EDITBOX_SCALE]->setEnabled(false);
					_ComboBox[COMBOBOX_MAINTYPE2]->clear();
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1080), 0);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1054), TYPE_TRAP);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1067), TYPE_TRAP + TYPE_CONTINUOUS);
					_ComboBox[COMBOBOX_MAINTYPE2]->addItem(dataManager.GetSysString(1070), TYPE_TRAP + TYPE_COUNTER);
					break;
				}

				}
			}
			}
		}				
		case COMBOBOX_SORTTYPE: {
					SortList();
					mainGame->env->setFocus(0);
					break;
		}
		default: break;
		}
		break;
	}
case irr::EET_MOUSE_INPUT_EVENT: {
	switch (event.MouseInput.Event) {
	case irr::EMIE_LMOUSE_PRESSED_DOWN: {
		irr::core::position2di mouse_pos(event.MouseInput.X, event.MouseInput.Y);
		irr::gui::IGUIElement* root = mainGame->env->getRootGUIElement();
		if (root->getElementFromPoint(mouse_pos) != root)
			break;
		if (wCategories->isVisible() || mainGame->wQuery->isVisible())
			break;
		if (hovered_pos == 0 || hovered_seq == -1)
			break;
		click_pos = hovered_pos;
		dragx = event.MouseInput.X;
		dragy = event.MouseInput.Y;
		draging_pointer = dataManager.GetCodePointer(hovered_code);
		if (draging_pointer == dataManager._datas.end())
			break;
		unsigned int limitcode = draging_pointer->second.alias ? draging_pointer->second.alias : draging_pointer->first;
		if (hovered_pos == 4) {
			int limit = 3;
			if (filterList->count(limitcode))
				limit = (*filterList)[limitcode];
			for (size_t i = 0; i < deckManager.current_deck.main.size(); ++i)
				if (deckManager.current_deck.main[i]->first == limitcode
					|| deckManager.current_deck.main[i]->second.alias == limitcode)
					limit--;
			for (size_t i = 0; i < deckManager.current_deck.extra.size(); ++i)
				if (deckManager.current_deck.extra[i]->first == limitcode
					|| deckManager.current_deck.extra[i]->second.alias == limitcode)
					limit--;
			for (size_t i = 0; i < deckManager.current_deck.side.size(); ++i)
				if (deckManager.current_deck.side[i]->first == limitcode
					|| deckManager.current_deck.side[i]->second.alias == limitcode)
					limit--;
			if (limit <= 0)
				break;
		}
		if (hovered_pos == 1)
			deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + hovered_seq);
		else if (hovered_pos == 2)
			deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + hovered_seq);
		else if (hovered_pos == 3)
			deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
		is_draging = true;
		break;
	}
	case irr::EMIE_LMOUSE_LEFT_UP: {
		if (!is_draging)
			break;
		if (!mainGame->is_siding) {
			if ((hovered_pos == 1 && (draging_pointer->second.type & 0x802040)) || (hovered_pos == 2 && !(draging_pointer->second.type & 0x802040)))
				hovered_pos = 0;
			if ((hovered_pos == 1 || (hovered_pos == 0 && click_pos == 1)) && deckManager.current_deck.main.size() < 60) {
				if (hovered_seq == -1)
					deckManager.current_deck.main.push_back(draging_pointer);
				else if (hovered_seq < (int)deckManager.current_deck.main.size() && hovered_pos)
					deckManager.current_deck.main.insert(deckManager.current_deck.main.begin() + hovered_seq, draging_pointer);
				else deckManager.current_deck.main.push_back(draging_pointer);
				is_draging = false;
			}
			else if ((hovered_pos == 2 || (hovered_pos == 0 && click_pos == 2)) && deckManager.current_deck.extra.size() < 15) {
				if (hovered_seq == -1)
					deckManager.current_deck.extra.push_back(draging_pointer);
				else if (hovered_seq < (int)deckManager.current_deck.extra.size() && hovered_pos)
					deckManager.current_deck.extra.insert(deckManager.current_deck.extra.begin() + hovered_seq, draging_pointer);
				else deckManager.current_deck.extra.push_back(draging_pointer);
				is_draging = false;
			}
			else if ((hovered_pos == 3 || (hovered_pos == 0 && click_pos == 3)) && deckManager.current_deck.side.size() < 15) {
				if (hovered_seq == -1)
					deckManager.current_deck.side.push_back(draging_pointer);
				else if (hovered_seq < (int)deckManager.current_deck.side.size() && hovered_pos)
					deckManager.current_deck.side.insert(deckManager.current_deck.side.begin() + hovered_seq, draging_pointer);
				else deckManager.current_deck.side.push_back(draging_pointer);
				is_draging = false;
			}
			else if (hovered_pos == 4)
				is_draging = false;
		}
		else {
			if ((hovered_pos == 1 && (draging_pointer->second.type & 0x802040)) || (hovered_pos == 2 && !(draging_pointer->second.type & 0x802040)) || hovered_pos == 4)
				hovered_pos = 0;
			if ((hovered_pos == 1 || (hovered_pos == 0 && click_pos == 1)) && deckManager.current_deck.main.size() < 65) {
				if (hovered_seq == -1)
					deckManager.current_deck.main.push_back(draging_pointer);
				else if (hovered_seq < (int)deckManager.current_deck.main.size() && hovered_pos)
					deckManager.current_deck.main.insert(deckManager.current_deck.main.begin() + hovered_seq, draging_pointer);
				else deckManager.current_deck.main.push_back(draging_pointer);
				is_draging = false;
			}
			else if ((hovered_pos == 2 || (hovered_pos == 0 && click_pos == 2)) && deckManager.current_deck.extra.size() < 20) {
				if (hovered_seq == -1)
					deckManager.current_deck.extra.push_back(draging_pointer);
				else if (hovered_seq < (int)deckManager.current_deck.extra.size() && hovered_pos)
					deckManager.current_deck.extra.insert(deckManager.current_deck.extra.begin() + hovered_seq, draging_pointer);
				else deckManager.current_deck.extra.push_back(draging_pointer);
				is_draging = false;
			}
			else if ((hovered_pos == 3 || (hovered_pos == 0 && click_pos == 3)) && deckManager.current_deck.side.size() < 20) {
				if (hovered_seq == -1)
					deckManager.current_deck.side.push_back(draging_pointer);
				else if (hovered_seq < (int)deckManager.current_deck.side.size() && hovered_pos)
					deckManager.current_deck.side.insert(deckManager.current_deck.side.begin() + hovered_seq, draging_pointer);
				else deckManager.current_deck.side.push_back(draging_pointer);
				is_draging = false;
			}
		}
		if (is_draging) {
			if (click_pos == 1)
				deckManager.current_deck.main.push_back(draging_pointer);
			else if (click_pos == 2)
				deckManager.current_deck.extra.push_back(draging_pointer);
			else if (click_pos == 3)
				deckManager.current_deck.side.push_back(draging_pointer);
		}
		is_draging = false;
		break;
	}
	case irr::EMIE_RMOUSE_LEFT_UP: {
		if (mainGame->is_siding) {
			if (is_draging)
				break;
			if (hovered_pos == 0 || hovered_seq == -1)
				break;
			draging_pointer = dataManager.GetCodePointer(hovered_code);
			if (draging_pointer == dataManager._datas.end())
				break;
			if (hovered_pos == 1) {
				if (deckManager.current_deck.side.size() < 20) {
					deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + hovered_seq);
					deckManager.current_deck.side.push_back(draging_pointer);
				}
			}
			else if (hovered_pos == 2) {
				if (deckManager.current_deck.side.size() < 20) {
					deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + hovered_seq);
					deckManager.current_deck.side.push_back(draging_pointer);
				}
			}
			else {
				if ((draging_pointer->second.type & 0x802040) && deckManager.current_deck.extra.size() < 20) {
					deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
					deckManager.current_deck.extra.push_back(draging_pointer);
				}
				if (!(draging_pointer->second.type & 0x802040) && deckManager.current_deck.main.size() < 64) {
					deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
					deckManager.current_deck.main.push_back(draging_pointer);
				}
			}
			break;
		}
		if (wCategories->isVisible() || mainGame->wQuery->isVisible())
			break;
		if (hovered_pos == 0 || hovered_seq == -1)
			break;
		if (!is_draging) {
			draging_pointer = dataManager.GetCodePointer(hovered_code);
			if (draging_pointer == dataManager._datas.end())
				break;
		}
		if (hovered_pos == 1) {
			if (!is_draging)
				deckManager.current_deck.main.erase(deckManager.current_deck.main.begin() + hovered_seq);
			else if (deckManager.current_deck.side.size() < 15) {
				deckManager.current_deck.side.push_back(draging_pointer);
				is_draging = false;
			}
		}
		else if (hovered_pos == 2) {
			if (!is_draging)
				deckManager.current_deck.extra.erase(deckManager.current_deck.extra.begin() + hovered_seq);
			else if (deckManager.current_deck.side.size() < 15) {
				deckManager.current_deck.side.push_back(draging_pointer);
				is_draging = false;
			}
		}
		else if (hovered_pos == 3) {
			if (!is_draging)
				deckManager.current_deck.side.erase(deckManager.current_deck.side.begin() + hovered_seq);
			else {
				if ((draging_pointer->second.type & 0x802040) && deckManager.current_deck.extra.size() < 15) {
					deckManager.current_deck.extra.push_back(draging_pointer);
					is_draging = false;
				}
				else if (!(draging_pointer->second.type & 0x802040) && deckManager.current_deck.main.size() < 60) {
					deckManager.current_deck.main.push_back(draging_pointer);
					is_draging = false;
				}
			}
		}
		else {
			if (is_draging) {
				if (deckManager.current_deck.side.size() < 15) {
					deckManager.current_deck.side.push_back(draging_pointer);
					is_draging = false;
				}
			}
			else {
				unsigned int limitcode = draging_pointer->second.alias ? draging_pointer->second.alias : draging_pointer->first;
				int limit = 3;
				if (filterList->count(limitcode))
					limit = (*filterList)[limitcode];
				for (size_t i = 0; i < deckManager.current_deck.main.size(); ++i)
					if (deckManager.current_deck.main[i]->first == limitcode
						|| deckManager.current_deck.main[i]->second.alias == limitcode)
						limit--;
				for (size_t i = 0; i < deckManager.current_deck.extra.size(); ++i)
					if (deckManager.current_deck.extra[i]->first == limitcode
						|| deckManager.current_deck.extra[i]->second.alias == limitcode)
						limit--;
				for (size_t i = 0; i < deckManager.current_deck.side.size(); ++i)
					if (deckManager.current_deck.side[i]->first == limitcode
						|| deckManager.current_deck.side[i]->second.alias == limitcode)
						limit--;
				if (limit <= 0)
					break;
				if ((draging_pointer->second.type & 0x802040) && deckManager.current_deck.extra.size() < 15) {
					deckManager.current_deck.extra.push_back(draging_pointer);
				}
				else if (!(draging_pointer->second.type & 0x802040) && deckManager.current_deck.main.size() < 60) {
					deckManager.current_deck.main.push_back(draging_pointer);
				}
			}
		}
		break;
	}
	case irr::EMIE_MOUSE_MOVED: {
		position2di pos = mainGame->Resize(event.MouseInput.X, event.MouseInput.Y, true);
		position2di mousepos(event.MouseInput.X, event.MouseInput.Y);
		s32 x = pos.X;
		s32 y = pos.Y;
		int pre_code = hovered_code;
		if (x >= 314 && x <= 794 && y >= 164 && y <= 435) {
			int lx = 10, px, py = (y - 164) / 68;
			hovered_pos = 1;
			if (deckManager.current_deck.main.size() > 40)
				lx = (deckManager.current_deck.main.size() - 41) / 4 + 11;
			if (x >= 750)
				px = lx - 1;
			else px = (x - 314) * (lx - 1) / 436;
			if (py*lx + px >= (int)deckManager.current_deck.main.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			}
			else {
				hovered_seq = py * lx + px;
				hovered_code = deckManager.current_deck.main[hovered_seq]->first;
			}
		}
		else if (x >= 314 && x <= 794 && y >= 466 && y <= 530) {
			int lx = deckManager.current_deck.extra.size();
			hovered_pos = 2;
			if (lx < 10)
				lx = 10;
			if (x >= 750)
				hovered_seq = lx - 1;
			else hovered_seq = (x - 314) * (lx - 1) / 436;
			if (hovered_seq >= (int)deckManager.current_deck.extra.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			}
			else {
				hovered_code = deckManager.current_deck.extra[hovered_seq]->first;
			}
		}
		else if (x >= 314 && x <= 794 && y >= 564 && y <= 628) {
			int lx = deckManager.current_deck.side.size();
			hovered_pos = 3;
			if (lx < 10)
				lx = 10;
			if (x >= 750)
				hovered_seq = lx - 1;
			else hovered_seq = (x - 314) * (lx - 1) / 436;
			if (hovered_seq >= (int)deckManager.current_deck.side.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			}
			else {
				hovered_code = deckManager.current_deck.side[hovered_seq]->first;
			}
		}
		else if (x >= 810 && x <= 995 && y >= 165 && y <= 626) {
			hovered_pos = 4;
			hovered_seq = (y - 165) / 66;
			if (scrFilter->getPos() + hovered_seq >= (int)results.size()) {
				hovered_seq = -1;
				hovered_code = 0;
			}
			else {
				hovered_code = results[scrFilter->getPos() + hovered_seq]->first;
			}
		}
		else {
			hovered_pos = 0;
			hovered_code = 0;
		}
		if (is_draging) {
			dragx = mousepos.X;
			dragy = mousepos.Y;
		}
		if (!is_draging && pre_code != hovered_code) {
			if (hovered_code) {
				mainGame->wInfoTab.ShowCardInfo(hovered_code);
			}
			if (pre_code)
				imageManager.RemoveTexture(pre_code);
		}
		break;
	}
		case irr::EMIE_MOUSE_WHEEL: {
			if(!scrFilter->isVisible())
				break;
			if(event.MouseInput.Wheel < 0) {
				if(scrFilter->getPos() < scrFilter->getMax())
					scrFilter->setPos(scrFilter->getPos() + 1);
			} else {
				if(scrFilter->getPos() > 0)
					scrFilter->setPos(scrFilter->getPos() - 1);
			}
			SEvent e = event;
			e.MouseInput.Event = irr::EMIE_MOUSE_MOVED;
			mainGame->device->postEventFromUser(e);
			break;
		}
		default: break;
		}
		break;
	}
	case irr::EET_KEY_INPUT_EVENT: {
		switch(event.KeyInput.Key) {
		case irr::KEY_KEY_R: {
			if (!event.KeyInput.PressedDown && !mainGame->HasFocus(EGUIET_EDIT_BOX))
				mainGame->textFont->setTransparency(true);
			break;
		}
		case irr::KEY_ESCAPE: {
			if (!mainGame->HasFocus(EGUIET_EDIT_BOX))
				mainGame->device->minimizeWindow();
			break;
		}
		default: break;
		}
		break;
	}
	default: break;
	}
	//check info tab too
	mainGame->wInfoTab.OnEvent(event);
	return false;
}

void GUIDeckEdit::FilterStart(){
	filter_type = _ComboBox[COMBOBOX_MAINTYPE]->getSelected();
				filter_type2 = _ComboBox[COMBOBOX_MAINTYPE2]->getItemData(_ComboBox[COMBOBOX_MAINTYPE2]->getSelected());
				filter_lm = _ComboBox[COMBOBOX_LIMIT]->getSelected();
				if(filter_type > 1) {
					FilterCards(true);
					return;
				}
				filter_attrib = _ComboBox[COMBOBOX_ATTRIBUTE]->getItemData(_ComboBox[COMBOBOX_ATTRIBUTE]->getSelected());
				filter_race = _ComboBox[COMBOBOX_RACE]->getItemData(_ComboBox[COMBOBOX_RACE]->getSelected());
				filter_setcode = _ComboBox[COMBOBOX_SETCODE]->getItemData(_ComboBox[COMBOBOX_SETCODE]->getSelected());
				const wchar_t* pstr = _editBox[EDITBOX_ATTACK]->getText();
				if(*pstr == 0) filter_atktype = 0;
				else {
					if(*pstr == L'=') {
						filter_atktype = 1;
						filter_atk = BufferIO::GetVal(pstr + 1);
					} else if(*pstr >= L'0' && *pstr <= L'9') {
						filter_atktype = 1;
						filter_atk = BufferIO::GetVal(pstr);
					} else if(*pstr == L'>') {
						if(*(pstr + 1) == L'=') {
							filter_atktype = 2;
							filter_atk = BufferIO::GetVal(pstr + 2);
						} else {
							filter_atktype = 3;
							filter_atk = BufferIO::GetVal(pstr + 1);
						}
					} else if(*pstr == L'<') {
						if(*(pstr + 1) == L'=') {
							filter_atktype = 4;
							filter_atk = BufferIO::GetVal(pstr + 2);
						} else {
							filter_atktype = 5;
							filter_atk = BufferIO::GetVal(pstr + 1);
						}
					} else if(*pstr == L'?') {
						filter_atktype = 6;
					} else filter_atktype = 0;
				}
				pstr = _editBox[EDITBOX_DEFENCE]->getText();
				if(*pstr == 0) filter_deftype = 0;
				else {
					if(*pstr == L'=') {
						filter_deftype = 1;
						filter_def = BufferIO::GetVal(pstr + 1);
					} else if(*pstr >= L'0' && *pstr <= L'9') {
						filter_deftype = 1;
						filter_def = BufferIO::GetVal(pstr);
					} else if(*pstr == L'>') {
						if(*(pstr + 1) == L'=') {
							filter_deftype = 2;
							filter_def = BufferIO::GetVal(pstr + 2);
						} else {
							filter_deftype = 3;
							filter_def = BufferIO::GetVal(pstr + 1);
						}
					} else if(*pstr == L'<') {
						if(*(pstr + 1) == L'=') {
							filter_deftype = 4;
							filter_def = BufferIO::GetVal(pstr + 2);
						} else {
							filter_deftype = 5;
							filter_def = BufferIO::GetVal(pstr + 1);
						}
					} else if(*pstr == L'?') {
						filter_deftype = 6;
					} else filter_deftype = 0;
				}
				pstr = _editBox[EDITBOX_LEVEL]->getText();
				if(*pstr == 0) filter_lvtype = 0;
				else {
					if(*pstr == L'=') {
						filter_lvtype = 1;
						filter_lv = BufferIO::GetVal(pstr + 1);
					} else if(*pstr >= L'0' && *pstr <= L'9') {
						filter_lvtype = 1;
						filter_lv = BufferIO::GetVal(pstr);
					} else if(*pstr == L'>') {
						if(*(pstr + 1) == L'=') {
							filter_lvtype = 2;
							filter_lv = BufferIO::GetVal(pstr + 2);
						} else {
							filter_lvtype = 3;
							filter_lv = BufferIO::GetVal(pstr + 1);
						}
					} else if(*pstr == L'<') {
						if(*(pstr + 1) == L'=') {
							filter_lvtype = 4;
							filter_lv = BufferIO::GetVal(pstr + 2);
						} else {
							filter_lvtype = 5;
							filter_lv = BufferIO::GetVal(pstr + 1);
						}
					} else filter_lvtype = 0;
				}
				pstr = _editBox[EDITBOX_SCALE]->getText();
				if (*pstr == 0) filter_scltype = 0;
				else {
					if (*pstr == L'=') {
						filter_scltype = 1;
						filter_scl = BufferIO::GetVal(pstr + 1);
					}
					else if (*pstr >= L'0' && *pstr <= L'9') {
						filter_scltype = 1;
						filter_scl = BufferIO::GetVal(pstr);
					}
					else if (*pstr == L'>') {
						if (*(pstr + 1) == L'=') {
							filter_scltype = 2;
							filter_scl = BufferIO::GetVal(pstr + 2);
						}
						else {
							filter_scltype = 3;
							filter_scl = BufferIO::GetVal(pstr + 1);
						}
					}
					else if (*pstr == L'<') {
						if (*(pstr + 1) == L'=') {
							filter_scltype = 4;
							filter_scl = BufferIO::GetVal(pstr + 2);
						}
						else {
							filter_scltype = 5;
							filter_scl = BufferIO::GetVal(pstr + 1);
						}
					}
					else filter_scltype = 0;
				}
}

void GUIDeckEdit::FilterCards(bool checkDescription) {
	results.clear();
	const wchar_t* pstr = _editBox[EDITBOX_KEYWORD]->getText();
	int trycode = BufferIO::GetVal(pstr);
	if(dataManager.GetData(trycode, 0)) {
		auto ptr = dataManager.GetCodePointer(trycode); // verified by GetData()
		results.push_back(ptr);
		scrFilter->setVisible(false);
		scrFilter->setPos(0);
		myswprintf(result_string, L"%d", results.size());
		return;
	}
	if(pstr[0] == 0)
		pstr = 0;
	auto strpointer = dataManager._strings.begin();
	for(code_pointer ptr = dataManager._datas.begin(); ptr != dataManager._datas.end(); ++ptr, ++strpointer) {
		const CardDataC& data = ptr->second;
		const CardString& text = strpointer->second;
		if(data.type & TYPE_TOKEN)
			continue;
		switch(filter_type) {
		case 1: {
			int type2 = data.type & 0xe03ef1;
			if(!(data.type & TYPE_MONSTER) || (filter_type2 == 0x21 && type2 != 0x21) || (data.type & filter_type2) != filter_type2)
				continue;
			if(filter_race && data.race != filter_race)
				continue;
			if(filter_attrib && data.attribute != filter_attrib)
				continue;
			if(filter_atktype) {
				if((filter_atktype == 1 && data.attack != filter_atk) || (filter_atktype == 2 && data.attack < filter_atk)
				        || (filter_atktype == 3 && data.attack <= filter_atk) || (filter_atktype == 4 && (data.attack > filter_atk || data.attack < 0))
				        || (filter_atktype == 5 && (data.attack >= filter_atk || data.attack < 0)) || (filter_atktype == 6 && data.attack != -2))
					continue;
			}
			if(filter_deftype) {
				if((filter_deftype == 1 && data.defence != filter_def) || (filter_deftype == 2 && data.defence < filter_def)
				        || (filter_deftype == 3 && data.defence <= filter_def) || (filter_deftype == 4 && (data.defence > filter_def || data.defence < 0))
				        || (filter_deftype == 5 && (data.defence >= filter_def || data.defence < 0)) || (filter_deftype == 6 && data.defence != -2))
					continue;
			}
			if(filter_lvtype) {
				if((filter_lvtype == 1 && data.level != filter_lv) || (filter_lvtype == 2 && data.level < filter_lv)
				        || (filter_lvtype == 3 && data.level <= filter_lv) || (filter_lvtype == 4 && data.level > filter_lv)
				        || (filter_lvtype == 5 && data.level >= filter_lv))
					continue;
			}
			if (filter_scltype) {
				if ((filter_scltype == 1 && data.lscale != filter_scl) || (filter_scltype == 2 && data.lscale < filter_scl)
					|| (filter_scltype == 3 && data.lscale <= filter_scl) || (filter_scltype == 4 && data.lscale > filter_scl)
					|| (filter_scltype == 5 && data.lscale >= filter_scl))
					continue;
			}
			break;
		}
		case 2: {
			if(!(data.type & TYPE_SPELL))
				continue;
			if(filter_type2 && data.type != filter_type2)
				continue;
			break;
		}
		case 3: {
			if(!(data.type & TYPE_TRAP))
				continue;
			if(filter_type2 && data.type != filter_type2)
				continue;
			break;
		}
		}
		if(filter_effect && !(data.category & filter_effect))
			continue;
		if (filter_setcode && filter_setcode != 0) {
			uint32 setcode = data.setcode & 0xffff;
			uint32 othersetcode = data.setcode >> 16;
			if (setcode != filter_setcode && othersetcode != filter_setcode)
				continue;
		}
		if(filter_lm) {
			if(filter_lm <= 3 && (!filterList->count(ptr->first) || (*filterList)[ptr->first] != filter_lm - 1))
				continue;
			//ocg
			if (filter_lm == 4 && !(data.ot & 0x1))
				continue;
			//tcg
			if (filter_lm == 5 && !(data.ot & 0x2))
				continue;
			//prerelease only
			if (filter_lm == 6 && !(data.ot & 0x4))
				continue;
			//ocg only
			if (filter_lm == 7 && (data.ot & 0x2))
				continue;
			//tcg only
			if (filter_lm == 8 && (data.ot & 0x1))
				continue;
		}
		if(pstr) {
			bool ok = CardNameCompare(text.name, pstr);
			if (!ok && checkDescription)
				ok = CardNameCompare(text.text, pstr);
			if (!ok)
				continue;
		}
		results.push_back(ptr);
	}
	myswprintf(result_string, L"%d", results.size());
	if(results.size() > 7) {
		scrFilter->setVisible(true);
		scrFilter->setMax(results.size() - 7);
		scrFilter->setPos(0);
	} else {
		scrFilter->setVisible(false);
		scrFilter->setPos(0);
	}
	SortList();
}

bool GUIDeckEdit::CardNameCompare(const wchar_t *sa, const wchar_t *sb)
{
	int i = 0;
	int j = 0;
	wchar_t ca;
	wchar_t cb;

	if (!sa || !sb)
		return false;
	while (sa[i])
	{
		ca = towupper(sa[i]);
		cb = towupper(sb[j]);
		if (ca == '-') ca = ' ';
		if (cb == '-') cb = ' ';
		if (ca == cb)
		{
			j++;
			if (!sb[j])
				return true;
		}
		else
			j = 0;
		i++;
	}
	return false;
}

void GUIDeckEdit::SortList() {
	auto left = results.begin();
	const wchar_t* pstr = _editBox[EDITBOX_KEYWORD]->getText();
	for (auto it = results.begin(); it != results.end(); ++it) {
		if (wcscmp(pstr, dataManager.GetName((*it)->first)) == 0) {
			std::iter_swap(left, it);
			++left;
		}
	}
	switch (_ComboBox[COMBOBOX_SORTTYPE]->getSelected()) {
	case 0:
		std::sort(left, results.end(), ClientCard::deck_sort_lv);
		break;
	case 1:
		std::sort(left, results.end(), ClientCard::deck_sort_atk);
		break;
	case 2:
		std::sort(left, results.end(), ClientCard::deck_sort_def);
		break;
	case 3:
		std::sort(left, results.end(), ClientCard::deck_sort_name);
		break;
	}
}

}
