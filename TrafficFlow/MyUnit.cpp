//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include <cstdint>
#include <fstream>
#include <cmath>
#include <set>

#include "MyUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
	Caption = L"ТранспортныйПоток - Безымянный";

	Table->Cells[0][0] = "№";
	Table->ColWidths[0] = 40;
	Table->Cells[1][0] = "N1";
	Table->Cells[2][0] = "ΔN1";
	Table->Cells[3][0] = "a";
	Table->Cells[4][0] = "b \\ T";

	InitialTable->Cells[0][0] = "№";
	InitialTable->Cells[0][1] = "1";
	InitialTable->Cells[1][0] = "N1";
	InitialTable->Cells[2][0] = "ΔN1";
	InitialTable->Cells[3][0] = "a";
	InitialTable->Cells[4][0] = "b";

	InitialTable->ColWidths[0] = 40;
	InitialTable->ColWidths[1] = 130;
	InitialTable->ColWidths[2] = 130;
	InitialTable->ColWidths[3] = 130;
	InitialTable->ColWidths[4] = 130;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CalcButtonClick(TObject *Sender)
{
    StatusBar1->SimpleText = "";

    // Проверка заполненности InitialTable
    for (int row = 1; row < InitialTable->RowCount; row++) {
        for (int col = 1; col <= 4; col++) {
            if (InitialTable->Cells[col][row].Trim().IsEmpty()) {
				StatusBar1->SimpleText = "Ошибка: Заполните все данные в строке " + IntToStr(row) + " таблицы.";
				return;
            }
        }
    }

    try {
        ValidateInputData();
	} catch (...) {
        return;
    }

    // Проверка заполненности Edit'ов
    if (EditT0->Text.Trim().IsEmpty()) {
		StatusBar1->SimpleText = L"Ошибка: Поле T0 не заполнено.";
        return;
    }

	if (EditTmax->Text.Trim().IsEmpty()) {
		StatusBar1->SimpleText = L"Ошибка: Поле Tmax не заполнено.";
        return;
    }

    if (EditDeltaT->Text.Trim().IsEmpty()) {
		StatusBar1->SimpleText = L"Ошибка: Поле ΔT не заполнено.";
        return;
    }

    Table->RowCount = 1;
    Table->ColCount = 5;
    Chart1->SeriesList->Clear();

    // Установка заголовков таблицы
    Table->Cells[0][0] = "№";
    Table->Cells[1][0] = "N1";
    Table->Cells[2][0] = "ΔN1";
    Table->Cells[3][0] = "a";
    Table->Cells[4][0] = "b \\ T";

	uint16_t T0 = StrToInt(EditT0->Text);
    uint16_t Tmax = StrToInt(EditTmax->Text);
    uint8_t deltaT = StrToInt(EditDeltaT->Text);

    int baseRow = 1;

    for (int i = 1; i < InitialTable->RowCount; i++) {
        try {
            int Num = StrToInt(InitialTable->Cells[0][i]);
            uint32_t N1 = StrToInt(InitialTable->Cells[1][i]);
            float deltaN1 = StrToFloat(InitialTable->Cells[2][i]);
            float a = StrToFloat(InitialTable->Cells[3][i]);
            float b = StrToFloat(InitialTable->Cells[4][i]);

            // Создание серии для графика
            TLineSeries *NtSeries = new TLineSeries(Chart1);
            Chart1->AddSeries(NtSeries);

            // Настройка легенды
            NtSeries->Title = "Набор " + IntToStr(i);
            Chart1->Legend->LegendStyle = lsSeries;
            Chart1->Legend->Visible = true;

            TColor seriesColor = NtSeries->Color;

            if (Table->RowCount <= baseRow)
                Table->RowCount = baseRow + 1;

            Table->Cells[0][baseRow] = IntToStr(Num);

            // Сохраняем цвет в первой колонке для наглядности
            Table->Objects[0][baseRow] = reinterpret_cast<TObject*>(seriesColor);

            // Заполнение исходных данных в таблице
            Table->Cells[1][baseRow] = IntToStr(static_cast<int>(N1));
            Table->Cells[2][baseRow] = FormatFloat("0.000", deltaN1);
            Table->Cells[3][baseRow] = FormatFloat("0.000", a);
            Table->Cells[4][baseRow] = FormatFloat("0.000", b);

            // Добавление данных по годам
            int colT = 5;
            uint32_t Nt = N1;
            float deltaNt;
            for (int T = T0; T <= Tmax; T += deltaT) {
                if (Table->ColCount <= colT)
                    Table->ColCount = colT + 1;

                Table->Cells[colT][0] = IntToStr(T);

                Table->Cells[colT][baseRow] = IntToStr(static_cast<int>(Nt));

                NtSeries->AddXY(T, Nt);

                if (T == 1) {
                    deltaNt = deltaN1;
                } else {
                    deltaNt = (a + b) / std::pow(T - 1, 1.0 / 3.0);
                }

                Nt = Nt * (1 + deltaNt / 100.0);

                colT++;
            }

            baseRow++;
        } catch (const Exception &e) {
            StatusBar1->SimpleText = "Ошибка обработки данных в наборе " + IntToStr(i) + ": " + e.Message;
            return;
        }
    }

	StatusBar1->SimpleText = "Расчёт завершён успешно.";
    Table->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::InitialData(const String& fileName)
{
	std::ofstream file(fileName.c_str(), std::ios::binary);
	if (!file)
	{
		ShowMessage("Ошибка при сохранении файла!");
		return;
	}

	file.exceptions(ios::failbit | ios::badbit);

	try
	{

		uint16_t T0 = EditT0->Text.IsEmpty() ? 0 : StrToInt(EditT0->Text);
		uint16_t Tmax = EditTmax->Text.IsEmpty() ? 0 : StrToInt(EditTmax->Text);
		uint8_t deltaT = EditDeltaT->Text.IsEmpty() ? 0 : StrToInt(EditDeltaT->Text);

		file.write((char*)&T0, sizeof(T0));
		file.write((char*)&Tmax, sizeof(Tmax));
		file.write((char*)&deltaT, sizeof(deltaT));

		uint32_t rowCount = InitialTable->RowCount - 1; // Исключаем заголовок строки
		file.write((char*)&rowCount, sizeof(rowCount));

		for (uint32_t row = 1; row <= rowCount; row++)
        {
            uint32_t N1 = 0;
			float deltaN1 = 0.0f;
            float a = 0.0f;
            float b = 0.0f;

			if (!InitialTable->Cells[1][row].Trim().IsEmpty())
				N1 = StrToInt(InitialTable->Cells[1][row]); // N1

			if (!InitialTable->Cells[2][row].Trim().IsEmpty())
				deltaN1 = StrToFloat(InitialTable->Cells[2][row]); // deltaN1

			if (!InitialTable->Cells[3][row].Trim().IsEmpty())
				a = StrToFloat(InitialTable->Cells[3][row]); // a

			if (!InitialTable->Cells[4][row].Trim().IsEmpty())
				b = StrToFloat(InitialTable->Cells[4][row]); // b

            file.write((char*)&N1, sizeof(N1));
			file.write((char*)&deltaN1, sizeof(deltaN1));
			file.write((char*)&a, sizeof(a));
			file.write((char*)&b, sizeof(b));
		}

		file.close();
		Caption = "ТранспортныйПоток - " + fileName;
		StatusBar1->SimpleText = L"Файл успешно сохранен: " + fileName;
	}
	catch (const Exception& e)
	{
		ShowMessage("Ошибка при сохранении данных: " + e.Message);
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveAsClick(TObject *Sender)
{
	String fileName;
	bool saveCanceled = false;

	while (!saveCanceled)
	{
        if (SaveDialog->Execute())
        {
            fileName = SaveDialog->FileName;

            // Проверка, существует ли файл
            if (FileExists(fileName))
            {
                // Вывод сообщения о перезаписи
                if (Application->MessageBox(L"Файл уже существует. Вы хотите перезаписать его?", L"Подтверждение перезаписи", MB_YESNO | MB_ICONQUESTION) == IDNO)
                {
                    continue;  // Если отмена, возвращаемся к началу цикла
                }
            }

			InitialData(fileName);
            Save->Enabled = false;
            saveCanceled = true; // Если все в порядке, выходим из цикла
        }
        else
        {
             saveCanceled = true; // Если диалог закрыт без сохранения, выходим из цикла
        }
    }
}
//---------------------------------------------------------------------------
bool __fastcall TForm1::saveData(bool showDialog) {
	String svdFileName = SaveDialog->FileName;


	if (showDialog) {                            // Если нужно показать диалог "Сохранить как..."
		if (!SaveDialog->Execute()) {
			return false;
		}
    }

	// Попытка сохранить файл
    try {
		InitialData(svdFileName);
		Save->Enabled = false;
	} catch (Exception &e) {
        // Если произошла ошибка сохранения, показать сообщение
        Application->MessageBox(
            Format(L"Ошибка сохранения файла \"%s\": \"%s\"",
				   ARRAYOFCONST((SaveDialog->FileName, e.Message))).w_str(),
            Application->Title.w_str(),
			MB_OK | MB_ICONERROR);
		SaveDialog->FileName = svdFileName; // Вернуть старое имя файла
		return false;
    }


	return true;
}


bool __fastcall TForm1::CheckChangesAndSave() {
	if (Save->Enabled) {        // Если кнопка "Сохранить" активна, значит есть несохранённые изменения
		String fileName;

		if (SaveDialog->FileName.IsEmpty()) {
			fileName = "Безымянный";
		} else {
			fileName = SaveDialog->FileName;
		}

        switch (Application->MessageBox(
			(String(L"Сохранить изменения в \"") + fileName + L"\"?").w_str(),
            Application->Title.w_str(),
			MB_YESNOCANCEL | MB_ICONQUESTION)) {

		case ID_YES:
			if (!saveData(SaveDialog->FileName.IsEmpty())) {
				return false;
			}
			break;

		case ID_CANCEL:
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::OpenClick(TObject *Sender)
{
	if (!CheckChangesAndSave()) {
		return; // Если пользователь решил не сохранять, прерываем действие
	}
	if (OpenDialog->Execute()) {

		for (int row = 1; row < InitialTable->RowCount; row++) {
			for (int col = 0; col < InitialTable->ColCount; col++) {
				InitialTable->Cells[col+1][row] = "";
			}
		}

		for (int row = 1; row < Table->RowCount; row++) {
			for (int col = 0; col < Table->ColCount; col++) {
				Table->Cells[col][row] = "";
			}
		}
		Table->RowCount = 1;
		Table->ColCount = 5;

		// Очистка графика и легенды
		Chart1->SeriesList->Clear();
		Chart1->Repaint();

		FileName = OpenDialog->FileName;
		std::ifstream file(FileName.c_str(), std::ios::binary);
		if (!file) {
			ShowMessage("Ошибка при открытии файла!");
			return;
		}

		try {
			uint16_t T0, Tmax;
			uint8_t deltaT;

			file.read((char*)&T0, sizeof(T0));
			file.read((char*)&Tmax, sizeof(Tmax));
			file.read((char*)&deltaT, sizeof(deltaT));

			EditT0->Text = IntToStr(T0);
			EditTmax->Text = IntToStr(Tmax);
			EditDeltaT->Text = IntToStr(deltaT);

			uint32_t rowCount;
			file.read((char*)&rowCount, sizeof(rowCount));

			InitialTable->RowCount = rowCount + 1;

			for (uint32_t row = 1; row <= rowCount; row++) {
				uint32_t N1;
				float deltaN1, a, b;

				file.read((char*)&N1, sizeof(N1));
				file.read((char*)&deltaN1, sizeof(deltaN1));
				file.read((char*)&a, sizeof(a));
				file.read((char*)&b, sizeof(b));

				InitialTable->Cells[0][row] = IntToStr(static_cast<int>(row));
				InitialTable->Cells[1][row] = IntToStr(static_cast<int>(N1));
				InitialTable->Cells[2][row] = FormatFloat("0.000", deltaN1);
				InitialTable->Cells[3][row] = FormatFloat("0.000", a);
				InitialTable->Cells[4][row] = FormatFloat("0.000", b);
			}

			file.close();
			Caption = "ТранспортныйПоток - " + FileName;
			StatusBar1->SimpleText = L"Файл успешно открыт: " + FileName;
            Save->Enabled = false;
			SaveDialog->FileName = OpenDialog->FileName;
			InitialTable->Cells[0][0] = "№";
			InitialTable->Cells[0][1] = "1";
			InitialTable->Cells[1][0] = "N1";
			InitialTable->Cells[2][0] = "ΔN1";
			InitialTable->Cells[3][0] = "a";
			InitialTable->Cells[4][0] = "b";

		} catch (const Exception &e) {
			file.close();
			ShowMessage("Ошибка при чтении данных: " + e.Message);
		}
	}



	if (InitialTable->RowCount > 2){
		DelRowButton->Enabled = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ValidateKeyPress_TEdit(TObject *Sender, System::WideChar &Key)
{

	if (!isdigit(Key) && Key != VK_BACK && Key != 3 && Key != 22 && Key != 1 && Key != 24 && Key != 26) {
		StatusBar1->SimpleText = L"'" + String(Key) + L"' - Недопустимый символ для целого числа!";
		Key = 0;
	}

	if (Sender == EditDeltaT) {

		if (Key == '0' && EditDeltaT->Text.IsEmpty()) {  // Если вводится 0 и поле будет содержать только 0 запрещаем ввод
			StatusBar1->SimpleText = L"Поле ΔT не может быть равно 0!";
			Key = 0;
        }
	}

	if (Sender == EditTmax) {

		if (Key == '0' && EditDeltaT->Text.IsEmpty()) {  // Если вводится 0 и поле будет содержать только 0 запрещаем ввод
			StatusBar1->SimpleText = L"Поле Tmax не может быть равно 0!";
			Key = 0;
		}
	}

	if (Sender == EditTmax || Sender == EditT0) {
        try {
            int t0 = EditT0->Text.ToInt();
            int tmax = EditTmax->Text.ToInt();

            if (Sender == EditTmax) {
                String tmaxPreview = EditTmax->Text + Key; // Предполагаемое значение Tmax после ввода
                tmax = tmaxPreview.ToInt();
            } else if (Sender == EditT0) {
                String t0Preview = EditT0->Text + Key; // Предполагаемое значение T0 после ввода
                t0 = t0Preview.ToInt();
            }

            if (tmax <= t0) {
                StatusBar1->SimpleText = L"Значение Tmax должно быть больше T0!";
                Key = 0;
            }
		} catch (...) {
            // Игнорируем исключения, вызванные временной некорректностью значения
		}
	}
}

void __fastcall TForm1::ValidateOnChange_TEdit(TObject *Sender)
{
	bool isValid = true;

	if (((TEdit*)Sender)->Text.IsEmpty()) {
		isValid = true;
	} else {
		try {
			StrToUInt(((TEdit*)Sender)->Text);
		}
		catch (...) {
			isValid = false;
		}
	}

	if (!isValid) {
		StatusBar1->SimpleText = L"Некорректное значение!";
	} else {
		StatusBar1->SimpleText = L"";
	}

	Save->Enabled = true;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TForm1::SaveClick(TObject *Sender)
{
	if (SaveDialog->FileName.IsEmpty())
	{
		SaveAsClick(Sender);
	}
	else
	{
		InitialData(FileName);
		Save->Enabled = false;
	}
}
//---------------------------------------------------------------------------*/


//---------------------------------------------------------------------------
void __fastcall TForm1::ValidateKeyPress_Table(TObject *Sender, System::WideChar &Key)
{
	int col = InitialTable->Col;

    switch (col) {
		case 1: // N1
			if (!isdigit(Key) && Key != VK_BACK && Key != 3 && Key != 22 && Key != 1 && Key != 24 && Key != 26) {
				StatusBar1->SimpleText = L"'" + String(Key) + L"' - Недопустимый символ для целого числа!";
				Key = 0;
			}
			break;

		case 2: // deltaN1
		case 3: // a
		case 4: // b
			if (!isdigit(Key) && Key != ',' && Key != VK_BACK && Key != 3 && Key != 22 && Key != 1 && Key != 24 && Key != 26) {
				StatusBar1->SimpleText = L"'" + String(Key) + L"' - Недопустимый символ для числа с плавающей запятой!";
				Key = 0;
			} else if (Key == ',') {
				String currentText = InitialTable->Cells[col][InitialTable->Row];
				if (currentText.Pos(',') > 0) {
					StatusBar1->SimpleText = L"Повторная запятая недопустима!";
					Key = 0;
				}
			}
			break;

        default:
			Key = 0;
			break;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ValidateOnChange_Table(TObject *Sender, System::LongInt ACol,
		  System::LongInt ARow, const UnicodeString Value)
{
	bool isValid = true;

	try {
		switch (ACol) {
			case 1: // N1
				if (!Value.IsEmpty()) {
					StrToUInt(Value);
				}
				break;

			case 2: // deltaN1
			case 3: // a
			case 4: // b
				if (!Value.IsEmpty()) {
					StrToFloat(Value);
				}
				break;

			default:
				isValid = false;
				break;
		}
	}
	catch (...) {
		isValid = false;
	}

	if (!isValid) {
		StatusBar1->SimpleText = L"Некорректное значение в таблице!";
	} else {
		StatusBar1->SimpleText = L"";
	}

	Save->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FileCreateClick(TObject *Sender)
{
	if (!CheckChangesAndSave()) {
		return;
	}

	SaveDialog->FileName = L"";

    EditT0->Text = "";
	EditTmax->Text = "";
    EditDeltaT->Text = "";

	for (int row = 1; row < InitialTable->RowCount; row++) {
		for (int col = 0; col < InitialTable->ColCount; col++) {
			InitialTable->Cells[col+1][row] = "";
		}
	}

	InitialTable->RowCount = 2;
	for (int col = 0; col < InitialTable->ColCount; col++) {
		InitialTable->Cells[col+1][1] = "";
	}

	for (int row = 1; row < Table->RowCount; row++) {
		for (int col = 0; col < Table->ColCount; col++) {
			Table->Cells[col][row] = "";
		}
	}
	Table->RowCount = 1;
	Table->ColCount = 5;

	// Очистка графика
	for (int i = 0; i < Chart1->SeriesCount(); i++) {
		Chart1->Series[i]->Clear();
	}

	Chart1->Legend->Visible = false;

	Save->Enabled = false;
	DelRowButton->Enabled = false;

    for (int i = 1; i < InitialTable->RowCount; ++i) {
		InitialTable->Cells[0][i] = IntToStr(i);
	}

	InitialTable->FixedRows=1;

    InitialTable->Cells[0][0] = "№";
	InitialTable->Cells[0][1] = "1";
	InitialTable->Cells[1][0] = "N1";
	InitialTable->Cells[2][0] = "ΔN1";
	InitialTable->Cells[3][0] = "a";
	InitialTable->Cells[4][0] = "b";
	Caption = "ТранспортныйПоток - Безымянный";
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCloseQuery(TObject *Sender, bool &CanClose)
{
	if (!CheckChangesAndSave())
	CanClose = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TableDrawCell(TObject *Sender, System::LongInt ACol, System::LongInt ARow,
									  TRect &Rect, TGridDrawState State)
{
	TStringGrid *grid = static_cast<TStringGrid*>(Sender);

	if (ARow == 0 && ACol == 4) // Заголовок столбца "b \\ T"
	{
		grid->Canvas->Brush->Color = clSilver;
		grid->Canvas->FillRect(Rect);

		grid->Canvas->Font->Color = clBlack;


		String textLeft = "b";
		String textMiddle = "\\";
		String textRight = "T";

		int leftTextWidth = grid->Canvas->TextWidth(textLeft);
		int middleTextWidth = grid->Canvas->TextWidth(textMiddle);
		int rightTextWidth = grid->Canvas->TextWidth(textRight);

		int leftX = Rect.Left + 2;
		int middleX = Rect.Left + (Rect.Width() - middleTextWidth) / 2;
		int rightX = Rect.Right - rightTextWidth - 6;

        grid->Canvas->TextOut(leftX, Rect.Top + 2, textLeft);
		grid->Canvas->TextOut(middleX, Rect.Top + 2, textMiddle);
        grid->Canvas->TextOut(rightX, Rect.Top + 2, textRight);
	}
	else if (ACol == 0 && ARow == 0){
		grid->Canvas->Brush->Color = clGray;
		grid->Canvas->FillRect(Rect);
        grid->Canvas->Font->Color = clBlack;
		grid->Canvas->TextOut(Rect.Left + 2, Rect.Top + 2, grid->Cells[ACol][ARow]);
	}
	else if (ACol == 0 && ARow > 0)
	{
		TColor cellColor = static_cast<TColor>(reinterpret_cast<intptr_t>(grid->Objects[ACol][ARow]));
		grid->Canvas->Brush->Color = cellColor;
		grid->Canvas->FillRect(Rect);
		grid->Canvas->Font->Color = clBlack;
		grid->Canvas->TextOut(Rect.Left + 2, Rect.Top + 2, grid->Cells[ACol][ARow]);
	}
	else if (ACol >= 1 && ACol <= 4 && ARow > 0)
	{
		grid->Canvas->Brush->Color = clWebLightgrey;
		grid->Canvas->FillRect(Rect);
		grid->Canvas->Font->Color = clBlack;
		grid->Canvas->TextOut(Rect.Left + 2, Rect.Top + 2, grid->Cells[ACol][ARow]);
	}
	else if (ACol >= 1 && ACol <= 4 && ARow == 0){
		grid->Canvas->Brush->Color = clSilver;
		grid->Canvas->FillRect(Rect);
		grid->Canvas->Font->Color = clBlack;
		grid->Canvas->TextOut(Rect.Left + 2, Rect.Top + 2, grid->Cells[ACol][ARow]);
	}

	if (ACol == 4)
	{
		grid->Canvas->Pen->Width = 2;
		grid->Canvas->Pen->Color = clBlack;

		grid->Canvas->MoveTo(Rect.Right - 1, Rect.Top);
		grid->Canvas->LineTo(Rect.Right - 1, Rect.Bottom);
	}

	if (ACol > 4)
	{
		grid->DefaultDrawing = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveTableClick(TObject *Sender)
{
	String fileName;
	bool saveCanceled = false;
	while(!saveCanceled)
	{
		if (SaveTableDialog->Execute())
		{
			fileName = SaveTableDialog->FileName;

			if (FileExists(fileName))
			{
                if (Application->MessageBox(L"Файл уже существует. Вы хотите перезаписать его?", L"Подтверждение перезаписи", MB_YESNO | MB_ICONQUESTION) == IDNO)
                {
					continue;
                }
            }

			std::ofstream file;
            try
            {
				file.open(fileName.c_str(), std::ios::out | std::ios::trunc);

                if (!file.is_open())
                {
                    ShowMessage(L"Ошибка: Не удалось открыть файл для записи.");
					break;
				}

                for (int row = 0; row < Table->RowCount; row++)
                {
                    for (int col = 0; col < Table->ColCount; col++)
                    {
                        String cellData = Table->Cells[col][row];

                        if (cellData.IsEmpty())
                        {
							cellData = "";
                        }

						file << AnsiString(cellData).c_str();

						if (col < Table->ColCount - 1)
						{
							file << ";\t";    // Разделитель для Excel
						}
					}
					file << "\n";
				}
				file.close();
				StatusBar1->SimpleText = L"Таблица успешно сохранена в файл: " + fileName;
			}
			catch (const std::exception &e)
			{
				if(file.is_open())
				  file.close();
				ShowMessage(L"Ошибка при сохранении файла: " + String(e.what()));
			}
			saveCanceled = true;
		}
		else {
			saveCanceled = true;
		}
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SaveGraphClick(TObject *Sender)
{
	String fileName;
    bool saveCanceled = false;

    while (!saveCanceled)
    {
        if (SaveGraphDialog->Execute())
        {
            fileName = SaveGraphDialog->FileName;
            int selectedFilterIndex = SaveGraphDialog->FilterIndex;
            String fileExtension;

            if (FileExists(fileName))
			{
                if (Application->MessageBox(L"Файл уже существует. Вы хотите перезаписать его?", L"Подтверждение перезаписи", MB_YESNO | MB_ICONQUESTION) == IDNO)
                {
					continue;
                }
            }

			if (selectedFilterIndex == 1) // Сохранение в PNG
            {
                fileExtension = ".png";
                TBitmap *bitmap = new TBitmap();
                try
                {
                    Chart1->SaveToBitmapFile(L"temp_graph.bmp");
                    bitmap->LoadFromFile(L"temp_graph.bmp");
                    TPngImage *png = new TPngImage();
                    try
                    {
                        png->Assign(bitmap);
                        // Добавляем расширение, если его нет
                        if (!SameText(ExtractFileExt(fileName), fileExtension))
                        {
                            fileName = ChangeFileExt(fileName, fileExtension);
                        }
                        png->SaveToFile(fileName); // Сохранить в PNG
                    }
                    __finally
                    {
                        delete png;
                    }

                    DeleteFile(L"temp_graph.bmp");
                    StatusBar1->SimpleText = L"График успешно сохранен в файл: " + fileName;
                }
                __finally
                {
                    delete bitmap;
                }
            }
            else if (selectedFilterIndex == 2) // Сохранение в EMF
            {
                fileExtension = ".emf";
                try
                {
					// Добавляем расширение, если его нет
                    if (!SameText(ExtractFileExt(fileName), fileExtension))
                    {
						fileName = ChangeFileExt(fileName, fileExtension);
                    }


                    TMetafile *metafile = new TMetafile();
                    try
                    {
						metafile->Enhanced = true;
                        TMetafileCanvas *canvas = new TMetafileCanvas(metafile, 0);
                        try
						{
							Chart1->Draw(canvas, Rect(0, 0, Chart1->Width, Chart1->Height));
                        }
                        __finally
						{
                            delete canvas;
                        }

                        metafile->SaveToFile(fileName);
                        StatusBar1->SimpleText = L"График успешно сохранен в файл: " + fileName;
                    }
                    __finally
                    {
                        delete metafile;
                    }
                }
                catch (...)
                {
                    Application->MessageBox(L"Ошибка при сохранении графика в формате EMF.", L"Ошибка", MB_ICONERROR);
                }
            }
            else
			{
                Application->MessageBox(L"Неверный формат файла.", L"Ошибка формата", MB_ICONERROR);
            }

			saveCanceled = true;
		}
        else
        {
			saveCanceled = true;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::TableKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
	if (Shift.Contains(ssShift)) // Проверяем, зажат ли Shift
	{
		switch (Key)
		{
			case VK_LEFT: // Стрелка влево
				Table->LeftCol = std::max(0, Table->LeftCol - 1);
				break;

			case VK_RIGHT: // Стрелка вправо
				Table->LeftCol = std::min(Table->ColCount - 1, Table->LeftCol + 1);
				break;
		}
	}
}
//---------------------------------------------------------------------------


void __fastcall TForm1::TableMouseWheelUp(TObject *Sender, TShiftState Shift, TPoint &MousePos,
		  bool &Handled)
{
	if (Shift.Contains(ssShift)) // Проверяем, зажат ли Shift
	{
		Table->LeftCol = std::max(0, Table->LeftCol - 1); // Прокрутка влево
		Handled = true;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TableMouseWheelDown(TObject *Sender, TShiftState Shift, TPoint &MousePos,
		  bool &Handled)
{
	if (Shift.Contains(ssShift)) // Проверяем, зажат ли Shift
	{
		Table->LeftCol = std::min(Table->ColCount - 1, Table->LeftCol + 1); // Прокрутка вправо
		Handled = true;
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ValidateInputData()
{
	for (int row = 1; row < InitialTable->RowCount; row++)
	{
       try
	   {
		  StrToInt(InitialTable->Cells[1][row]);    // N1
		  StrToFloat(InitialTable->Cells[2][row]);  // deltaN1
		  StrToFloat(InitialTable->Cells[3][row]);  //  a
		  StrToFloat(InitialTable->Cells[4][row]);  //  b
        }
       catch (const Exception &e)
		{
			StatusBar1->SimpleText = "Ошибка: Некорректные данные в строке " + IntToStr(row);
			throw;
		}
    }

	try
    {
        StrToInt(EditT0->Text);
	}
    catch (const Exception &e)
    {
		StatusBar1->SimpleText = "Ошибка: Некорректные данные в поле T0.";
		throw;
    }

    try
    {
		StrToInt(EditTmax->Text);
    }
	catch (const Exception &e)
    {
	   StatusBar1->SimpleText = "Ошибка: Некорректные данные в поле Tmax.";
	   throw;
	}

	try
	{
		StrToInt(EditDeltaT->Text);
	}
	catch (const Exception &e)
	{
		StatusBar1->SimpleText = "Ошибка: Некорректные данные в поле ΔT.";
		throw;
	}
	if(StrToInt(EditDeltaT->Text) == 0){
		StatusBar1->SimpleText = L"Поле ΔT не может быть равно 0!";
		throw;
	}
}

void __fastcall TForm1::CheckBox1Click(TObject *Sender)
{
	if (CheckBox1->Checked) {
		InitialTable->Options = InitialTable->Options << goEditing;
	} else {
		InitialTable->Options = InitialTable->Options >> goEditing;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::InitialTableKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)
{
    static String oldValue;

    if (Key == VK_RETURN) {
		Key = 0;

        if (InitialTable->EditorMode) {
			oldValue = InitialTable->Cells[InitialTable->Col][InitialTable->Row];
		}

        InitialTable->EditorMode = !InitialTable->EditorMode;

        if (InitialTable->EditorMode) {
            oldValue = InitialTable->Cells[InitialTable->Col][InitialTable->Row];
        }
    }

    if (Key == VK_ESCAPE) {
		Key = 0;


        if (InitialTable->EditorMode) {
			InitialTable->Cells[InitialTable->Col][InitialTable->Row] = oldValue;
			InitialTable->EditorMode = false;
		}
	}
}

//---------------------------------------------------------------------------
void __fastcall TForm1::DelRowMenuClick(TObject *Sender)
{
	int topRow = InitialTable->Selection.Top;
	int bottomRow = InitialTable->Selection.Bottom;

	if (topRow == -1 || bottomRow == -1)
		return;

	String message = L"Вы уверены, что хотите удалить строки: ";

	for (int i = topRow; i <= bottomRow; ++i) {
		if (i < InitialTable->RowCount) {
			message += IntToStr(i) + L", ";
		}
	}

	if (message.Length() > 2) {
		message = message.SubString(1, message.Length() - 2);
	}

	message += L"?";


	int result = Application->MessageBox(
		message.w_str(),
		Application->Title.w_str(),
		MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2); // кнопка "Нет" по умолчанию


	if (result == IDYES) {
        for (int i = bottomRow; i >= topRow; --i) {
			if (i < InitialTable->RowCount) {
				// Сдвигаем строки вверх
				for (int j = i; j < InitialTable->RowCount - 1; ++j) {
                    for (int col = 0; col < InitialTable->ColCount; ++col) {
						InitialTable->Cells[col][j] = InitialTable->Cells[col][j + 1];
                    }
				}
				// Очищаем последнюю строку после сдвига
				if (InitialTable->RowCount > 0) {
                    for (int col = 0; col < InitialTable->ColCount; ++col) {
						InitialTable->Cells[col][InitialTable->RowCount - 1] = "";
                    }
				}

				// Уменьшаем количество строк
                InitialTable->RowCount--;
			}
        }
		// Перенумеровываем первую колонку
		for (int i = 1; i < InitialTable->RowCount; i++) {
			InitialTable->Cells[0][i] = IntToStr(i);
        }
	}

	if (InitialTable->RowCount <= 1) {
		DelRowMenu->Enabled = false;
        DelRowButton->Enabled = false;
		StatusBar1->SimpleText = L"В таблице должна быть хотя бы одна фиксированная строка.";

		CheckBox1->Enabled = false;
		CheckBox1->Checked = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::AddRowMenuClick(TObject *Sender)
{
    int newRow = InitialTable->RowCount;
    InitialTable->RowCount++;

    InitialTable->Cells[0][newRow] = IntToStr(newRow);

    if (InitialTable->RowCount >= 1) {
        DelRowMenu->Enabled = true;
        DelRowButton->Enabled = true;
        CheckBox1->Enabled = true;
        CheckBox1->Checked = true;
        InitialTable->FixedRows = 1;
	}

    InitialTable->Row = newRow;
}

//---------------------------------------------------------------------------
int __fastcall TForm1::CalGridColWidth(TStringGrid* Grid, int Col) {
	TCanvas* Canvas = new TCanvas();
	try {
		Canvas->Handle = Grid->Canvas->Handle;
		Canvas->Font = Grid->Font;
		int maxWidth = 0;
		for (int row = 0; row < Grid->RowCount; ++row) {
			TRect textRect;
            DrawTextW(Canvas->Handle, Grid->Cells[Col][row].c_str(), -1, &textRect, DT_CALCRECT | DT_SINGLELINE);
            maxWidth = std::max(maxWidth, textRect.Width());
		}
		return maxWidth + 10;
    }
    __finally {
		delete Canvas;
	}
}

void __fastcall TForm1::InsertRowMenuClick(TObject *Sender)
{
	int currentRow = InitialTable->Row;
	if (currentRow < 0) return;

	int newRow = currentRow + 1;

	InitialTable->RowCount++;

	for (int i = InitialTable->RowCount - 2; i >= newRow; i--) {
		for (int j = 0; j < InitialTable->ColCount; j++) {
			InitialTable->Cells[j][i + 1] = InitialTable->Cells[j][i];
		}
	}

	for (int j = 0; j < InitialTable->ColCount; j++) {
		InitialTable->Cells[j][newRow] = "";
	}

	InitialTable->Cells[0][newRow] = IntToStr(newRow + 1);

	for (int i = 1; i < InitialTable->RowCount; i++) {
		InitialTable->Cells[0][i] = IntToStr(i);
	}

	InitialTable->Row = newRow;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::InitialTableDblClick(TObject *Sender)
{
    TPoint p;
    GetCursorPos(&p);
    TStringGrid* grid = dynamic_cast<TStringGrid*>(Sender);

    if (grid) {
        p = grid->ScreenToClient(p);


		if (GetCursor() == Screen->Cursors[crHSplit]) {           // курсор на краю?
            TGridCoord gc = grid->MouseCoord(p.X - 4, p.Y);
            int width = CalGridColWidth(grid, gc.X);
            if (width < 5) {
                width = 5;
            }
            grid->ColWidths[gc.X] = width;
        }
		else {
			TGridCoord gc = grid->MouseCoord(p.X, p.Y);           // иначе по заголовку
			if (gc.Y == 0) {
				int col = gc.X;

				static int lastSortedCol = -1;          // если уже отсортирован
                static bool ascending = true;


				if (lastSortedCol == col) {              // смена напрвления
                    ascending = !ascending;
				} else {
                    ascending = true;
                }

				if (lastSortedCol != -1 && lastSortedCol != col) {                  // удаляем стрелочку с пердыдущего
					String prevHeaderText = grid->Cells[lastSortedCol][0];

					int arrowPos = prevHeaderText.Pos("▲");
                    if (arrowPos > 0) {
                        prevHeaderText = prevHeaderText.SubString(1, arrowPos - 1);
                    }
                    else {
						arrowPos = prevHeaderText.Pos("▼");
						if (arrowPos > 0) {
							prevHeaderText = prevHeaderText.SubString(1, arrowPos - 1);
                        }
                    }
                    grid->Cells[lastSortedCol][0] = prevHeaderText;
                }

                lastSortedCol = col;

				SortGridByColumn(grid, col, ascending);

				String arrow = ascending ? "▲" : "▼";
                String headerText = grid->Cells[col][0];

				int arrowPos = headerText.Pos("▲");
                if (arrowPos > 0) {
                    headerText = headerText.SubString(1, arrowPos - 1);
                }
                else {
					arrowPos = headerText.Pos("▼");
                    if (arrowPos > 0) {
                        headerText = headerText.SubString(1, arrowPos - 1);
                    }
                }

				headerText = headerText + arrow;

				grid->Cells[col][0] = headerText;
            }
        }
    }
}



void TForm1::SortGridByColumn(TStringGrid *grid, int col, bool ascending)
{
	int rowCount = grid->RowCount;
	bool swapped;

	do {
		swapped = false;
		for (int i = 1; i < rowCount - 1; i++) {
			String cell1 = grid->Cells[col][i];
			String cell2 = grid->Cells[col][i + 1];

			bool condition = (ascending ? cell1 > cell2 : cell1 < cell2);

			if (condition) {

				for (int j = 1; j < grid->ColCount; j++) {
					String temp = grid->Cells[j][i];           // строки меняем местами
					grid->Cells[j][i] = grid->Cells[j][i + 1];
					grid->Cells[j][i + 1] = temp;
				}
				swapped = true;
			}
		}
	} while (swapped);
}

