with Ada.Text_IO;
with Ada.Command_Line;
with Ada.Strings.Unbounded;
with Ada.Strings.Fixed;
with Interfaces.C;
with Interfaces.C.Strings;
with C_Slides;
with System;

procedure Slides_Main is
   use Ada.Text_IO;
   use Ada.Command_Line;
   use Ada.Strings.Unbounded;
   use Interfaces.C;
   use Interfaces.C.Strings;
   use C_Slides;

   procedure Print_Help is
   begin
      Put_Line ("Uso: slides_main [opciones] presentacion.md");
      Put_Line ("");
      Put_Line ("Opciones:");
      Put_Line ("  -p, --palette <name>    Elegir paleta (dark, rose, monokai, nord, light)");
      Put_Line ("  -f, --font-family <str> Definir tipografía (ej. 'Arial', 'JetBrains Mono')");
      Put_Line ("  -s, --font-scale <num>  Escalar tamaño de fuentes (ej. 1.2)");
      Put_Line ("  -e, --export            Exportar slides a PNG");
      Put_Line ("  -er, --export-res <WxH> Resolución de exportación (ej. 1920x1080)");
      Put_Line ("  -sl, --slide <num>      Seleccionar slide específico para exportar (0-index)");
      Put_Line ("  -h, --help              Mostrar esta ayuda");
   end Print_Help;

   MD_Path      : Unbounded_String;
   Palette_Name : Unbounded_String;
   Font_Family  : Unbounded_String;
   Font_Scale   : double := 1.0;
   Export_PNG   : Boolean := False;
   Target_Slide : int := -1;
   Export_W     : int := 1080;
   Export_H     : int := 1080;

   Arg_Idx : Positive := 1;
begin
   if Argument_Count = 0 then
      Print_Help;
      return;
   end if;

   while Arg_Idx <= Argument_Count loop
      declare
         Arg : constant String := Argument (Arg_Idx);
      begin
         if Arg = "--palette" or Arg = "-p" then
            if Arg_Idx + 1 <= Argument_Count then
               Arg_Idx := Arg_Idx + 1;
               Palette_Name := To_Unbounded_String (Argument (Arg_Idx));
            end if;
         elsif Arg = "--font-family" or Arg = "-f" then
            if Arg_Idx + 1 <= Argument_Count then
               Arg_Idx := Arg_Idx + 1;
               Font_Family := To_Unbounded_String (Argument (Arg_Idx));
            end if;
         elsif Arg = "--font-scale" or Arg = "-s" then
            if Arg_Idx + 1 <= Argument_Count then
               Arg_Idx := Arg_Idx + 1;
               Font_Scale := double (Long_Float'Value (Argument (Arg_Idx)));
            end if;
         elsif Arg = "--export" or Arg = "-e" then
            Export_PNG := True;
         elsif Arg = "--export-res" or Arg = "-er" then
            if Arg_Idx + 1 <= Argument_Count then
               Arg_Idx := Arg_Idx + 1;
               declare
                  Res_Str : constant String := Argument (Arg_Idx);
                  X_Pos   : constant Natural := Ada.Strings.Fixed.Index (Res_Str, "x");
               begin
                  if X_Pos > 0 then
                     Export_W := int (Integer'Value (Res_Str (Res_Str'First .. X_Pos - 1)));
                     Export_H := int (Integer'Value (Res_Str (X_Pos + 1 .. Res_Str'Last)));
                  end if;
               exception
                  when others =>
                     Put_Line ("Error al parsear resolución. Usando 1080x1080.");
               end;
            end if;
         elsif Arg = "--slide" or Arg = "-sl" then
            if Arg_Idx + 1 <= Argument_Count then
               Arg_Idx := Arg_Idx + 1;
               Target_Slide := int (Integer'Value (Argument (Arg_Idx)));
            end if;
         elsif Arg = "--help" or Arg = "-h" then
            Print_Help;
            return;
         else
            MD_Path := To_Unbounded_String (Arg);
         end if;
      end;
      Arg_Idx := Arg_Idx + 1;
   end loop;

   if Length (MD_Path) = 0 then
      Print_Help;
      return;
   end if;

   declare
      C_Path : chars_ptr := New_String (To_String (MD_Path));
      S      : Slider_Ptr := Slider_Load (C_Path);
   begin
      Free (C_Path);
      if S = Slider_Ptr (System.Null_Address) then
         Put_Line ("Error al cargar slides.");
         return;
      end if;

      if Length (Palette_Name) > 0 then
         declare
            C_Palette : chars_ptr := New_String (To_String (Palette_Name));
         begin
            Slider_Set_Theme (S, C_Palette);
            Free (C_Palette);
         end;
      end if;

      if Length (Font_Family) > 0 then
         declare
            C_Font : chars_ptr := New_String (To_String (Font_Family));
         begin
            Slider_Set_Font_Family (S, C_Font);
            Free (C_Font);
         end;
      end if;

      if Font_Scale /= 1.0 then
         Slider_Set_Font_Scale (S, Font_Scale);
      end if;

      declare
         N_Slides : constant int := Slider_Get_Count (S);
         Theme_Name : constant String := Value (Slider_Get_Theme_Name (S));
         Font_Name  : constant String := Value (Slider_Get_Font_Family (S));
         Scale      : constant double := Slider_Get_Font_Scale (S);
      begin
         Put_Line ("[slides-ada] " & Integer (N_Slides)'Image & " slide(s) cargados (tema: " & Theme_Name & 
                  ", font: " & Font_Name & ", scale: " & Long_Float (Scale)'Image & ")");

         if Export_PNG then
            declare
               Start_Idx : int := 0;
               End_Idx   : int := N_Slides;
            begin
               if Target_Slide >= 0 then
                  Start_Idx := Target_Slide;
                  End_Idx   := Target_Slide + 1;
               end if;

               if Start_Idx < 0 or Start_Idx >= N_Slides then
                  Put_Line ("Error: Slide fuera de rango.");
                  Slider_Free (S);
                  return;
               end if;

               for I in Integer (Start_Idx) .. Integer (End_Idx) - 1 loop
                  declare
                     Filename : constant String := "slide_ada_" & I'Image & ".png";
                     C_Filename : chars_ptr := New_String (Filename);
                     Ret : int;
                  begin
                     Ret := Slider_Export_PNG (S, int (I), C_Filename, Export_W, Export_H);
                     Free (C_Filename);
                     if Ret = 0 then
                        Put_Line ("Exportado: " & Filename & " (" & int'Image (Export_W) & "x" & int'Image (Export_H) & ")");
                     else
                        Put_Line ("Fallo al exportar: " & Filename);
                     end if;
                  end;
               end loop;

               Slider_Free (S);
               return;
            end;
         end if;

         Put_Line ("Iniciando backend Linux (X11)...");
         declare
            Ret : constant int := Backend_Run (S);
         begin
            Slider_Free (S);
         end;
      end;
   end;
end Slides_Main;
