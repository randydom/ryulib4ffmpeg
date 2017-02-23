unit _fmMain;

interface

uses
  RyuMPEG, WaveOut,
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.ExtCtrls;

type
  TfmMain = class(TForm)
    OpenDialog: TOpenDialog;
    Panel1: TPanel;
    btOpen: TButton;
    ScrollBox: TScrollBox;
    Image: TImage;
    procedure btOpenClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    FStream : pointer;
    FVideo : pointer;
    FAudio : pointer;
    FFrame : pointer;
    FAudioBuffer : pointer;
    FWaveOut : TWaveOut;
  public
  end;

var
  fmMain: TfmMain;

implementation

{$R *.dfm}

procedure TfmMain.btOpenClick(Sender: TObject);
var
  iError, iAudioSize : integer;
begin
  if not OpenDialog.Execute then Exit;

  FStream := OpenStream(OpenDialog.FileName, iError);
  if iError <> 0 then raise Exception.Create('Error: open_stream');

  Caption := IntToStr( get_duration(FStream) );

  FVideo := open_video(FStream, iError);
  if iError <> 0 then raise Exception.Create('Error: open_video');

  Image.Picture.Bitmap.Width  := get_video_width (FVideo);
  Image.Picture.Bitmap.Height := -get_video_height(FVideo);

  FAudio := open_audio(FStream, iError);
  if iError <> 0 then raise Exception.Create('Error: open_audio');

  FWaveOut.Channels := get_channels(FAudio);
  FWaveOut.SampleRate := get_sample_rate(FAudio);
  FWaveOut.Start;

  FFrame := read_frame(FStream);
  while FFrame <> nil do begin
    try
      Caption := IntToStr( get_frame_position(FFrame) );
      case get_frame_type(FFrame) of
        VIDEO_PACKET: decode_video(FVideo, FFrame, Image.Picture.Bitmap.ScanLine[get_video_height(FVideo)-1]);

        AUDIO_PACKET: begin
          decode_audio(FAudio, FFrame, FAudioBuffer, iAudioSize);
          if iAudioSize > 0 then FWaveOut.Play(FAudioBuffer, iAudioSize);
        end;
      end;
    finally
      release_frame(FFrame);
    end;

    Image.Repaint;
    Application.ProcessMessages;

    FFrame := read_frame(FStream);
  end;
end;

procedure TfmMain.FormCreate(Sender: TObject);
begin
  Image.Picture.Bitmap.PixelFormat := pf32bit;

  GetMem(FAudioBuffer, 1024 * 16);

  FWaveOut := TWaveOut.Create(Self);
end;

end.
