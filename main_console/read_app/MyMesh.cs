using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;


//using System.Runtime.InteropServices;
using System.Threading;
using System;

using System.Reflection;
using System.Text;
using System.Security;
using SkpInterface;
//使用call_me接口；
public class MyMesh : MonoBehaviour
{

    //public FBXImporter fbxImporter;
    //用于存储绘制三角形的顶点坐标  
    private Vector3[] vertices;
    //用于记录绘制三角形所需要的顶点ID顺序  
    private int[] triangles;
    //记录顶点数  
    private int count = 0;
    //定义Mesh  
    private Mesh mesh;
    //定义一个链表用于记录所有点的坐标  
    private List<Vector3> list;

    Mesh m_mesh;
    float m_speed = 10.0f;
    float m_rotationY = 0.0f;

    void Start()
    {


        MeshInfo tSingleMeshInfo = MeshInfo.getInstance();
        //liuliang测试直接运行场景测试
        tSingleMeshInfo.getmeshData().FileType = "SKP";
        //测试带纹理模型
        //tSingleMeshInfo.meshData.PathName = Application.dataPath + "/Resources/uv_test1cube/TextureBox.skp";
        // tSingleMeshInfo.meshData.FilePath = Application.dataPath + "/Resources/uv_test1cube";

        //测试颜色模型
        tSingleMeshInfo.meshData.PathName = "D:/sketchup/test_skp_file/empty.skp";
        //liuliang测试直接运行场景测试
        if (tSingleMeshInfo.getmeshData().FileType == "DEF")
        {
            GameObject newObject = Instantiate(Resources.Load("DefaultMesh")) as GameObject;
            newObject.transform.localScale = new Vector3(1.0f, 1.0f, 1.0f);
            newObject.transform.position = new Vector3(0.0f, 0.0f, 0.0f);
            return;
        }

        if (tSingleMeshInfo.getmeshData().FileType == "SKP")
        {
            Debug.Log("Start loading " + tSingleMeshInfo.meshData.PathName);
            IntPtr _skp_exporter = IntPtr.Zero;

            var ptr = Marshal.StringToHGlobalAnsi(tSingleMeshInfo.meshData.PathName);
            _skp_exporter = SkpInterface.SkpDLL.GetExporter(ptr);
            Marshal.FreeHGlobal(ptr);

            Debug.Log("加载SKP模型 :" + tSingleMeshInfo.meshData.PathName);
            

            if (_skp_exporter != IntPtr.Zero)
            {
                unsafe
                {
                    //group num test,actually we have (_group_num+1) in the su scene.
                    //Groups are in range (0,_group_num)
                    //-----------------------------------------------------------
                    int _group_num = SkpInterface.SkpDLL.GetGroupNum(_skp_exporter);
                    Debug.Log("group num =" + _group_num);

                    //get facing camera faces id list
                    //0 2 4,means we have 2(size -1) groups.face id 0,1 are in a group, 2,3 are in a group.
                    int facing_cam_grp_size = SkpInterface.SkpDLL.GetFacingCameraIdSize(_skp_exporter);
                    int[] facing_cam_id= new int[facing_cam_grp_size];
                    SkpInterface.SkpDLL.GetFacingCameraId(_skp_exporter, facing_cam_id);
                    Debug.Log("Facing camera group size : "+facing_cam_grp_size);
                    for(int i = 0; i < facing_cam_grp_size; i++)
                    {
                        Debug.Log(facing_cam_id[i]);
                    }

                    //get face data---------------------------------------------------
                    int _vertex_num = 0;
                    int _face_num = 0;
                    SkpInterface.SkpDLL.GetFaceDSize(_skp_exporter,
                                                        0,
                                                        out _vertex_num,
                                                        out _face_num);
                    Debug.Log("vertex num : " + _vertex_num);
                    Debug.Log("face num : " + _face_num);

                    float[] _vertices = new float[_vertex_num * 3];
                    int[] _vertex_num_per_face = new int[_face_num];
                    float[] _face_normal = new float[_face_num * 3];

                    SkpInterface.SkpDLL.GetFace(_skp_exporter, 0,
                                                    _vertices,
                                                    _vertex_num_per_face,
                                                    _face_normal);

                    //uv test-----------------------------------------------------
                    int uv_num = 0;//u v list length
                    uv_num = SkpInterface.SkpDLL.GetFaceUVDSize(_skp_exporter, 0, true);
                    Debug.Log("uv_num ： " + uv_num);


                    float[] u = new float[uv_num];//u value list
                    float[] v = new float[uv_num];//v value list
                    SkpInterface.SkpDLL.GetFaceUV(_skp_exporter, 0,
                                                    true,//true for front uv,false for back uv
                                                    u,
                                                    v);
                    int uv_index = 0;
                    while (uv_index < uv_num)
                    {
                        Debug.Log("u v: " + u[uv_index]+" "+v[uv_index]);
                        uv_index++;
                    }






                    ////material test---------------------------------------------------
                    int mat_num = SkpInterface.SkpDLL.GetMaterialNum(_skp_exporter);//get the num of materials
                    Debug.Log("material num ： " + mat_num);

                    int mat_index = 0;
                    while (mat_index < mat_num)
                    {
                        StringBuilder mat_name = new StringBuilder(100);

                        SkpInterface.SkpDLL.GetMaterialNameByID(_skp_exporter, mat_index, mat_name); //get the material name by id
                        Debug.Log("material name " + mat_index + " : " + mat_name.ToString());

                        int data_size = 0;
                        data_size = SkpInterface.SkpDLL.GetTexPixelDSize(_skp_exporter, mat_index);//get the texture pixel size
                        float[] pixel_data = new float[data_size];

                        bool has_color = false;
                        float[] color = new float[3];
                        bool has_alpha = false;
                        float alpha = 1;
                        bool has_texture = false;
                        int bits_per_pixel = 0;
                        int width = 0, height = 0;
                        float t_scale=0, s_scale=0;
                        SkpInterface.SkpDLL.GetMaterialData(_skp_exporter,
                                                                mat_index,
                                                                out has_color,
                                                                color,
                                                                out has_alpha,
                                                                out alpha,
                                                                out has_texture,
                                                                out s_scale,
                                                                out t_scale,
                                                                out bits_per_pixel,
                                                                out width,
                                                                out height,
                                                                pixel_data);

                        Debug.Log("Material id -------" + mat_index);

                        if (has_color)
                        {
                            Debug.Log("Color: " + mat_index + " " + color[0] + " " + color[1] + " " + color[2]);
                        }
                        if (has_alpha)
                        {
                            Debug.Log("Alpha: " + mat_index + " " + alpha);
                        }
                        if (has_texture)
                        {
                            //如果纹理存在，应该打印出纹理路径
                            Debug.Log("Texture: " + mat_index + " width:" + width + " height: " + height + " data size: " + data_size + " bits per pixel: " + bits_per_pixel);
                            if (bits_per_pixel / 8 == 3)
                                Debug.Log("RGB");
                            else
                                Debug.Log("RGBA");
                            Debug.Log("Texture color first 4: " + pixel_data[0] + " " + pixel_data[1] + " " + pixel_data[2] + " " + pixel_data[3]);
                            Debug.Log("Texture color last 4: " + pixel_data[data_size - 3] + " " + pixel_data[data_size - 2] + " " + pixel_data[data_size - 1] + " " + pixel_data[data_size - 1]);

                        }
                        mat_index++;

                    }








                    //----------------------------------------------------------------------
                    int[] front_mat_id = new int[_face_num];//front material
                    int[] back_mat_id = new int[_face_num];//
                    SkpInterface.SkpDLL.GetMaterialIDPerFace(_skp_exporter, 0,
                                                                front_mat_id,
                                                                back_mat_id);


                    Debug.Log("mat id num: " + _face_num);
                    int face_id = 0;
                    while (face_id < _face_num)
                    {
                        Debug.Log("Material for face id: " + face_id + " Front mat id: " + front_mat_id[face_id] + " Back mat id: " + back_mat_id[face_id]);
                        face_id++;
                    }
                    Debug.Log("mesh.triangles:" + mesh.triangles.Length);

                    //mesh.
                    Debug.Log("Sketchup model load ends.");

                }

                //print something...
            }


            SkpInterface.SkpDLL.ReleaseExporter(_skp_exporter);
        }

        if (tSingleMeshInfo.getmeshData().FileType == "FBX")
        {
            // fbxImporter.ImportAllFBX(tSingleMeshInfo.meshData.PathName);
        }
        return;
    }


    void Update()
    {

    }

    public int[] GetRangeArray(int[] SourceArray, int StartIndex, int EndIndex)
    {
        int[] result = new int[EndIndex - StartIndex + 1];
        for (int i = 0; i <= EndIndex - StartIndex; i++)
            result[i] = SourceArray[i + StartIndex];
        return result;
    }
    IEnumerator IGetSKPTexture(string filePath, int materialIndex)
    {
        WWW www = new WWW("file:///C:/Users/Administrator/Desktop/uv_test1cube/Wood_Floor_Dark.jpg");
        yield return www.isDone;
        this.gameObject.GetComponent<MeshRenderer>().materials[materialIndex].mainTexture = www.texture;
    }
}
