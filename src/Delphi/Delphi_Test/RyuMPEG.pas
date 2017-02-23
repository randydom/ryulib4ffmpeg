unit RyuMPEG;

interface

uses
  Classes, SysUtils;

const
  ERROR_GENERAL = -1;
  ERROR_CAN_NOT_OPEN_FILE = -2;
  ERROR_CAN_NOT_FIND_STREAMINFO = -3;
  ERROR_CAN_NOT_FIND_VIDOESTREAM = -4;
  ERROR_CAN_NOT_FIND_AUDIOSTREAM = -5;
  ERROR_CAN_NOT_OPEN_VIDEOCODEC = -6;
  ERROR_CAN_NOT_OPEN_AUDIOCODEC = -7;
  ERROR_CAN_NOT_OPEN_AUDIO_RESAMPLER = -8;

  UNKNOWN_PACKET = 0;
  VIDEO_PACKET = 1;
  AUDIO_PACKET = 2;

function open_stream(filename:PAnsiChar; var error:integer):pointer;
          cdecl; external 'ryumpeg.dll';

procedure close_stream(stream:pointer);
          cdecl; external 'ryumpeg.dll' delayed;

function read_frame(stream:pointer):pointer;
          cdecl; external 'ryumpeg.dll' delayed;

procedure release_frame(frame:pointer);
          cdecl; external 'ryumpeg.dll' delayed;

function open_audio(stream:pointer; var error:integer):pointer;
          cdecl; external 'ryumpeg.dll' delayed;

procedure close_audio(handle:pointer);
          cdecl; external 'ryumpeg.dll' delayed;

procedure decode_audio(handle,frame:pointer; data_out:pointer; var size_out:integer);
          cdecl; external 'ryumpeg.dll' delayed;

function open_video(stream:pointer; var error:integer):pointer;
          cdecl; external 'ryumpeg.dll' delayed;

procedure close_video(handle:pointer);
          cdecl; external 'ryumpeg.dll' delayed;

function decode_video(handle,frame,bitmap:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

procedure flush_video_buffer(handle:pointer);
          cdecl; external 'ryumpeg.dll' delayed;

procedure set_position(stream:pointer; position:integer);
          cdecl; external 'ryumpeg.dll' delayed;

function get_duration(stream:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function get_video_width(handle:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function get_video_height(handle:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function get_sample_rate(handle:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function get_channels(handle:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function get_frame_type(frame:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function get_frame_position(frame:pointer):integer;
          cdecl; external 'ryumpeg.dll' delayed;

function OpenStream(const AFilename:string; var error:integer):pointer;

implementation

function OpenStream(const AFilename:string; var error:integer):pointer;
begin
  Result := open_stream(PAnsiChar(AnsiString(AFilename)), error);
end;

end.
