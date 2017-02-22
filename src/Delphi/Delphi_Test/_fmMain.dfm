object fmMain: TfmMain
  Left = 0
  Top = 0
  Caption = 'ryumpeg test'
  ClientHeight = 754
  ClientWidth = 889
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 889
    Height = 41
    Align = alTop
    BevelOuter = bvNone
    TabOrder = 0
    object btOpen: TButton
      Left = 10
      Top = 9
      Width = 75
      Height = 25
      Caption = 'btOpen'
      TabOrder = 0
      OnClick = btOpenClick
    end
  end
  object ScrollBox: TScrollBox
    Left = 0
    Top = 41
    Width = 889
    Height = 713
    Align = alClient
    TabOrder = 1
    object Image: TImage
      Left = 0
      Top = 0
      Width = 105
      Height = 105
      AutoSize = True
    end
  end
  object OpenDialog: TOpenDialog
    Filter = 'All files|*.*'
    Left = 60
    Top = 200
  end
end
