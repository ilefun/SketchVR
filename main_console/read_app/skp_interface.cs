using System;
using System.Runtime.InteropServices;
using System.Reflection;
using System.Collections;
using System.Text;
using System.Security;
using Microsoft.Win32.SafeHandles;

namespace SkpInterface
{

    public partial class SkpDLL
    {
        //get exporter
        [DllImport("SkpReader", ExactSpelling = true,CharSet = CharSet.Ansi, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern IntPtr GetExporter(IntPtr from_file,bool both_sides=true);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void ReleaseExporter(IntPtr exporter);


        //group data func
        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetGroupNum(IntPtr exporter);


        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void GetFaceDSize(IntPtr exporter,
                                                    int group_id,
                                                    out int vertex_num,
                                                    out int face_num);
        //get face data
        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetFace(IntPtr exporter,
                                                int group_id,
                                                float[] vertices,
                                                int[] vertex_num_per_face,
                                                float[] face_normal);

        
        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetFacingCameraIdSize(IntPtr exporter);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetFacingCameraId(IntPtr exporter, 
                                                            int id,
                                                            out int start_face,
                                                            out int end_face);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetFacingCameraDirection(IntPtr exporter,
                                                                int id,
                                                                float[] direction);



        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetSceneSize(IntPtr exporter);
        
        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void GetSceneData(IntPtr exporter,
                                                    int id, 
                                                    [Out, MarshalAs(UnmanagedType.LPStr)] StringBuilder scene_name, 
                                                    float[] position,
                                                    float[] target,
                                                    float[] upvector);



        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetFaceUVDSize(IntPtr exporter,
                                                    int group_id,
                                                    bool front_or_back);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetFaceUV(IntPtr exporter,
                                                    int group_id,
                                                    bool front_or_back,
                                                    float[] u,
                                                    float[] v);


        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetMaterialNum(IntPtr exporter);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetMaterialNameByID(IntPtr exporter, int id, [Out, MarshalAs(UnmanagedType.LPStr)] StringBuilder mat_name);



        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetTexPixelDSize(IntPtr exporter, int id);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetMaterialData(IntPtr exporter,
                                                int material_id,
                                                out bool has_color,
                                                float[] color,
                                                out bool has_alpha,
                                                out float alpha,
                                                out bool has_texture,
                                                out float tex_sscale,
                                                out float tex_tscale,
                                                out int origin_bits_per_pixel,
                                                out int bits_per_pixel,
                                                out int width,
                                                out int height,
                                                float[] pixel_data );

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetMaterialIDPerFace(IntPtr exporter,
                                int group_id,
                                int[] front_material_id_per_face,
                                int[] back_material_id_per_face);

    }
}