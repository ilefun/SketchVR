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
        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool ReleaseDoubleHandle(IntPtr handle);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool ReleaseIntHandle(IntPtr handle);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool ReleaseBoolHandle(IntPtr handle);

        public class VectorDoubleSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
        {
            public VectorDoubleSafeHandle()
                : base(true)
            {
            }
            protected override bool ReleaseHandle()
            {
                return ReleaseDoubleHandle(handle);
            }
        };

        public class VectorIntSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
        {
            public VectorIntSafeHandle()
                : base(true)
            {
            }
            protected override bool ReleaseHandle()
            {
                return ReleaseIntHandle(handle);
            }
        };

        public class VectorBoolSafeHandle : SafeHandleZeroOrMinusOneIsInvalid
        {
            public VectorBoolSafeHandle()
                : base(true)
            {
            }
            protected override bool ReleaseHandle()
            {
                return ReleaseBoolHandle(handle);
            }
        }

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern IntPtr GetExporter(string from_file, string to_folder);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern void ReleaseExporter(IntPtr exporter);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetFaceData(IntPtr exporter,
                                                out double* vertices,
                                                out int vertex_num,
                                                out int* vertex_num_per_face,
                                                out int face_num,
                                                out int* face_vertex_index,
                                                out VectorDoubleSafeHandle vertices_handle,
                                                out VectorIntSafeHandle vertices_face_handle,
                                                out VectorIntSafeHandle face_vindex_handle);


        public static unsafe void GetSkpFaceData(IntPtr exporter,
                                        out double* vertices,
                                        out int vertex_num,
                                        out int* vertex_num_per_face,
                                        out int face_num,
                                        out int* face_vertex_index)
        {
            VectorDoubleSafeHandle _vertices_handle;
            VectorIntSafeHandle _vertices_face_handle;
            VectorIntSafeHandle _face_vindex_handle;

            if (!GetFaceData(exporter,
                            out vertices,
                            out vertex_num,
                            out vertex_num_per_face,
                            out face_num,
                            out face_vertex_index,
                            out _vertices_handle,
                            out _vertices_face_handle,
                            out _face_vindex_handle))
            {
                throw new InvalidOperationException();
            }
        }


        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetFaceUV(IntPtr exporter,
                                                    bool front_or_back,
                                                    out double* u,
                                                    out double* v,
                                                    out int uv_num,
                                                    out int* uv_id,
                                                    out int uv_id_num,
                                                    out VectorIntSafeHandle uv_id_handle,
                                                    out VectorDoubleSafeHandle u_handle,
                                                    out VectorDoubleSafeHandle v_handle);

        public static unsafe void GetSkpFaceUV(IntPtr exporter,
                                                bool front_or_back,
                                                out double* u,
                                                out double* v,
                                                out int uv_num,
                                                out int* uv_id,
                                                out int uv_id_num)
        {
            VectorDoubleSafeHandle _u_handle;
            VectorDoubleSafeHandle _v_handle;
            VectorIntSafeHandle _uv_id_handle;

            if (!GetFaceUV(exporter,
                            front_or_back,
                            out u,
                            out v,
                            out uv_num,
                            out uv_id,
                            out uv_id_num,
                            out _uv_id_handle,
                            out _u_handle,
                            out _v_handle))
            {
                throw new InvalidOperationException();
            }
        }


        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern int GetMaterialNum(IntPtr exporter);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetMaterialNameByID(IntPtr exporter, int id, [Out, MarshalAs(UnmanagedType.LPStr)] StringBuilder mat_name);

        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetMaterialData(IntPtr exporter,
                            int material_id,
                            out bool has_color,
                            double[] color,
                            out bool has_alpha,
                            out double alpha,
                            out bool has_texture,
                            [Out, MarshalAs(UnmanagedType.LPStr)] StringBuilder texture_path,
                            out double tex_sscale,
                            out double tex_tscale);



        [DllImport("SkpReader", ExactSpelling = true, CallingConvention = CallingConvention.Cdecl)]
        public static unsafe extern bool GetMaterialIDPerFace(IntPtr exporter,
                                out int* front_material_id_per_face,
                                out int* back_material_id_per_face,
                                out int face_num,
                                out VectorIntSafeHandle front_mat_handle,
                                out VectorIntSafeHandle back_mat_handle);

        public static unsafe void GetSkpMaterialIDPerFace(IntPtr exporter,
                                out int* front_material_id_per_face,
                                out int* back_material_id_per_face,
                                out int face_num)

        {
            VectorIntSafeHandle _front_mat_id_handle;
            VectorIntSafeHandle _back_mat_id_handle;

            if (!GetMaterialIDPerFace(exporter,
                            out front_material_id_per_face,
                            out back_material_id_per_face,
                            out face_num,
                            out _front_mat_id_handle,
                            out _back_mat_id_handle))
            {
                throw new InvalidOperationException();
            }
        }
    }
}