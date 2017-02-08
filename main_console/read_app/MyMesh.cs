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
        tSingleMeshInfo.meshData.PathName = "D:/sketchup/cube_group.skp";
        tSingleMeshInfo.meshData.FilePath = "D:/sketchup/reader_data";
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
            _skp_exporter = SkpInterface.SkpDLL.GetExporter(tSingleMeshInfo.meshData.PathName);
            
            if (_skp_exporter.ToInt32() > 0)
            {
                unsafe
                {
                    //group hierachy test
                    int _group_num = SkpInterface.SkpDLL.GetGroupNum(_skp_exporter);
                    Debug.Log("group num =" + _group_num);


                    //get group xform value
                    double[] xform = new double[16];
                    SkpInterface.SkpDLL.GetGroupTransformById(_skp_exporter, -1, xform);
                    Debug.Log("xform : " + xform[0] + " " + xform[1] + " " + xform[2] + " " + xform[3]);
                    Debug.Log("xform : " + xform[4] + " " + xform[5] + " " + xform[6] + " " + xform[7]);
                    Debug.Log("xform : " + xform[8] + " " + xform[9] + " " + xform[10] + " " + xform[11]);
                    Debug.Log("xform : " + xform[12] + " " + xform[13] + " " + xform[14] + " " + xform[15]);



                    //get group children
                    int* _children_id;
                    int _children_num=0;
                    SkpInterface.SkpDLL.GetSkpGroupChildrenById(_skp_exporter,
                                                                -1,
                                                                out _children_id,
                                                                out _children_num);
                    Debug.Log("children num of -1 : " + _children_num);
                    //test children xform of group -1
                    //int grp_id = 0;
                    //while (grp_id < _children_num)
                    //{
                    //    Debug.Log("xform debug id : "+grp_id+"----------------");
                    //    SkpInterface.SkpDLL.GetGroupTransformById(_skp_exporter, *(_children_id+grp_id), xform);
                    //    Debug.Log("xform " + grp_id + " : " + xform[0] + " " + xform[1] + " " + xform[2] + " " + xform[3]);
                    //    Debug.Log("xform " + grp_id + " : " + xform[4] + " " + xform[5] + " " + xform[6] + " " + xform[7]);
                    //    Debug.Log("xform " + grp_id + " : " + xform[8] + " " + xform[9] + " " + xform[10] + " " + xform[11]);
                    //    Debug.Log("xform " + grp_id + " : " + xform[12] + " " + xform[13] + " " + xform[14] + " " + xform[15]);
                    //    grp_id++;
                    //}

                    //get face data
                    double* _vertices;
                    int _vertex_num;
                    int* _vertex_num_per_face;
                    int _face_num;
                    double* _face_normal;

                    SkpInterface.SkpDLL.GetSkpFace(_skp_exporter, -1,
                                out _vertices,
                                out _vertex_num,
                                out _vertex_num_per_face,
                                out _face_num,
                                out _face_normal);
                    Debug.Log("vertex num : " + _vertex_num);
                    Debug.Log("face num : " + _face_num);




                    //uv test-----------------------------------------------------
                    double* u;//u value list
                    double* v;//v value list
                    int uv_num = 0;//u v list length
                    SkpInterface.SkpDLL.GetSkpFaceUV(_skp_exporter, -1,
                                                    true,//true for front uv,false for back uv
                                                    out u,
                                                    out v,
                                                    out uv_num);
                    Debug.Log("uv_num ： " + uv_num);





                    ////material test---------------------------------------------------
                    int mat_num = SkpInterface.SkpDLL.GetMaterialNum(_skp_exporter);//get the num of materials
                    Debug.Log("material num ： " + mat_num);

                    int mat_index = 0;
                    while (mat_index < mat_num)
                    {
                        StringBuilder mat_name = new StringBuilder(100);

                        SkpInterface.SkpDLL.GetMaterialNameByID(_skp_exporter, mat_index, mat_name); //get the material name by id
                        //string str = new string(materialName);
                        Debug.Log("material name "+mat_index+" : "+ mat_name.ToString());

                        // bool has_color = false;
                        // double []color = new double[3];
                        // bool has_alpha = false;
                        // double alpha = 1;
                        // bool has_texture = false;
                        // StringBuilder tex_path = new StringBuilder(100);
                        // double tex_sscale = 1.0;
                        // double tex_tscale = 1.0;
                        // StringBuilder tex_p = new StringBuilder();
                        // SkpInterface.SkpDLL.GetSkpMaterialData(_skp_exporter,
                        //          mat_index,
                        //          out has_color,
                        //          color,
                        //          out has_alpha,
                        //          out alpha,
                        //          out has_texture,
                        //          tex_path,
                        //          out tex_sscale,
                        //          out tex_tscale);

                        // //Debug.Log("materail -------"+ mat_index);

                        // if (has_color)
                        // {
                        //       Debug.Log("Color -------"+ mat_index + " "+color[0]+" "+color[1]+" "+color[2]);

                        // }
                        // if (has_alpha)
                        // {
                        //     Debug.Log("Alpha-------" + mat_index + " " + alpha);
                        // }
                        // if (has_texture)
                        // {
                        //     //如果纹理存在，应该打印出纹理路径
                        //     Debug.Log("Texture------" + mat_index + " " + tex_path.ToString());
                        // }
                        mat_index++;


                    }


                    //----------------------------------------------------------------------
                    //int* front_mat_id;//front material
                    //int* back_mat_id;//
                    //int id_num = 0;
                    //SkpInterface.SkpDLL.GetSkpMaterialIDPerFace(_skp_exporter, -1,
                    //                                            out front_mat_id,
                    //                                            out back_mat_id,
                    //                                            out id_num);


                    //int mat_index_per_face = 0;
                    //Debug.Log("mat id per face:" + id_num);
                    //Debug.Log("mesh.triangles:" + mesh.triangles.Length);

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
