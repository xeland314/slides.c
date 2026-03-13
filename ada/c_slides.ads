with Interfaces.C.Strings;
use Interfaces.C.Strings;
with Interfaces.C;
use Interfaces.C;
with System;

package C_Slides is

   type Slider_Ptr is new System.Address;

   function Slider_Load (Path : chars_ptr) return Slider_Ptr;
   pragma Import (C, Slider_Load, "slider_load");

   function Backend_Run (S : Slider_Ptr) return int;
   pragma Import (C, Backend_Run, "backend_run");

   procedure Slider_Free (S : Slider_Ptr);
   pragma Import (C, Slider_Free, "slider_free");

   function Slider_Get_Count (S : Slider_Ptr) return int;
   pragma Import (C, Slider_Get_Count, "slider_get_count");

   function Slider_Export_PNG (S : Slider_Ptr; Index : int; Path : chars_ptr; W, H : int) return int;
   pragma Import (C, Slider_Export_PNG, "slider_export_png");

   -- Property accessors
   procedure Slider_Set_Theme (S : Slider_Ptr; Theme_Name : chars_ptr);
   pragma Import (C, Slider_Set_Theme, "slider_set_theme");

   procedure Slider_Set_Font_Family (S : Slider_Ptr; Font_Family : chars_ptr);
   pragma Import (C, Slider_Set_Font_Family, "slider_set_font_family");

   procedure Slider_Set_Font_Scale (S : Slider_Ptr; Font_Scale : double);
   pragma Import (C, Slider_Set_Font_Scale, "slider_set_font_scale");

   function Slider_Get_Theme_Name (S : Slider_Ptr) return chars_ptr;
   pragma Import (C, Slider_Get_Theme_Name, "slider_get_theme_name");

   function Slider_Get_Font_Family (S : Slider_Ptr) return chars_ptr;
   pragma Import (C, Slider_Get_Font_Family, "slider_get_font_family");

   function Slider_Get_Font_Scale (S : Slider_Ptr) return double;
   pragma Import (C, Slider_Get_Font_Scale, "slider_get_font_scale");

end C_Slides;
