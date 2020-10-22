{$reference 'System.dll'}

uses System, System.Collections;


procedure ShowHelp(binName: String);
begin
  WriteFormat('Usage: {0} ifile ofile' #10
              '  ifile   The input file (.def)' #10
              '  ofile   The output file (.h)' #10 #10, binName);
end;


begin
  if ParamCount() <> 2 then begin
    Writeln('This program takes exactly 2 parameters');
    ShowHelp(ParamStr(0));
    Halt(1);
  end;
  
  var ifile, ofile : text;
  
  Assign(ifile, ParamStr(1));
  Assign(ofile, ParamStr(2));
  
  ifile.Reset();
  //ofile.Rewrite();
  
  foreach var line : String in ifile.Lines() do begin
    if (line.StartsWith('#') or (line.length = 0)) then continue;
    Writeln(line);
    
    var tmp : array of String;
    tmp := line.Split(Arr(':'), 2);
    tmp[0] := tmp[0].Trim();
    tmp[1] := tmp[1].Trim();
    
    var opnum : Integer;
    try
      opnum := Convert.ToInt32(tmp[0], 10);
    except 
      on System.FormatException do
        opnum := Convert.ToInt32(tmp[0], 16);
    end;
    
    tmp := tmp[1].Split(Arr('('), 2);
    tmp[0] := tmp[0].Trim();
    tmp[1] := tmp[1].Trim();
    
    Assert(tmp[1].EndsWith(')'));
    tmp[1] := tmp[1].Substring(0, tmp[1].length - 1);
    
    var opname: String;
    opname := tmp[0];
    
    if (tmp[1].Contains(':')) then begin
      tmp := tmp[1].Split(Arr(':'), 2);
      tmp[0] := tmp[0].Trim();
      tmp[1] := tmp[1].Trim();
      
      WritelnFormat('> {1} = 0x{0:X2}; {2} {3}', opnum, opname, tmp[0], tmp[1]);
    end else begin
      WritelnFormat('> {1} = 0x{0:X2};', opnum, opname);
    end;
    
    
  end;
  
  
  Close(ifile);
  //Close(ofile);
end.