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
public class MyMesh : MonoBehaviour {

    public Material meshUsematerial;
    private Color[] meshColorList;
    private String[] meshTextureList;
    private String[] meshMaterailNameList;
    private Vector2[] meshMaterailTextureScaleList;
    float m_speed = 10.0f;
    float m_rotationY = 0.0f;
    void Start()
    {

    
        MeshInfo tSingleMeshInfo = MeshInfo.getInstance();
        //liuliang测试直接运行场景测试
        if (tSingleMeshInfo.getmeshData().FileType == "DEF")
        {
            GameObject  newObject = Instantiate(Resources.Load("DefaultMesh")) as GameObject;
            newObject.transform.localScale = new Vector3(1.0f, 1.0f, 1.0f);
            newObject.transform.position = new Vector3(0.0f, 0.0f, 0.0f);
            return;
        }
      
        if (tSingleMeshInfo.getmeshData().FileType == "SKP")
        {

            IntPtr _skp_exporter = IntPtr.Zero;
            _skp_exporter = SkpInterface.SkpDLL.GetExporter(tSingleMeshInfo.meshData.PathName);
            Debug.Log("加载SKP模型 :" + tSingleMeshInfo.meshData.PathName);
        

            if (_skp_exporter.ToInt32() > 0)
            {
               unsafe
                {
                    //构建模型组
                   int nGroupNum = SkpInterface.SkpDLL.GetGroupNum(_skp_exporter);
                    if(nGroupNum>0)
                    {
                         for(int i = 0; i < nGroupNum; i++)
                        {
                            if(i==126)
                            {
                                int a = 0;
                            }
                            Debug.Log("构建第一个组 ：" + i);
                            creatGameMesh(i, _skp_exporter, this.gameObject);
                           
                        }
                    }
                    else
                    {
                        creatGameMesh(-1, _skp_exporter,this.gameObject);
                    }
                   // gameObject.AddComponent<MeshFilter>();
                   // gameObject.AddComponent<MeshRenderer>();
                   // Mesh mesh = GetComponent<MeshFilter>().mesh;
                   // mesh.Clear();
                    Debug.Log("--1.先进行材质分析----------------------------------------------------------------------------------------//");
                   


                }

                //print something...
            }

            SkpInterface.SkpDLL.ReleaseExporter(_skp_exporter);
           
        }

        if (tSingleMeshInfo.getmeshData().FileType == "FBX")
        {
           // fbxImporter.ImportAllFBX(tSingleMeshInfo.meshData.PathName);
        }
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
   
    void creatGameMesh(int nGroupId, IntPtr _skp_exporter, GameObject partent)
    {
        GameObject gameMesh = new GameObject();
        gameMesh.AddComponent<MeshFilter>();
        gameMesh.AddComponent<MeshRenderer>();
        gameMesh.transform.parent = partent.transform;
        Mesh mesh = gameMesh.GetComponent<MeshFilter>().mesh;
        mesh.Clear();


   
        unsafe
        {
            double[] groupTransform=new double[16];
            SkpInterface.SkpDLL.GetGroupTransformById(_skp_exporter, nGroupId, groupTransform);
           

            Matrix4x4 mat =new Matrix4x4();
            mat.SetColumn(0,new Vector4((float)groupTransform[0], (float)groupTransform[1], (float)groupTransform[2], (float)groupTransform[3]));
            mat.SetColumn(1, new Vector4((float)groupTransform[4], (float)groupTransform[5], (float)groupTransform[6], (float)groupTransform[7]));
            mat.SetColumn(2, new Vector4((float)groupTransform[8], (float)groupTransform[9], (float)groupTransform[10], (float)groupTransform[11]));
            mat.SetColumn(3, new Vector4((float)groupTransform[12], (float)groupTransform[13], (float)groupTransform[14], (float)groupTransform[15]));

            //缩放

            Vector3 scale;
            scale.x = new Vector4(mat.m00, mat.m10, mat.m20, mat.m30).magnitude;
            scale.y = new Vector4(mat.m01, mat.m11, mat.m21, mat.m31).magnitude;
            scale.z = new Vector4(mat.m02, mat.m12, mat.m22, mat.m32).magnitude;
            gameMesh.transform.localScale = scale;

            //旋转
            Vector3 forward;
            forward.x = mat.m02;
            forward.y = mat.m12;
            forward.z = mat.m22;

            Vector3 upwards;
            upwards.x = mat.m01;
            upwards.y = mat.m11;
            upwards.z = mat.m21;

            gameMesh.transform.rotation= Quaternion.Euler(90, 0, 0)*Quaternion.LookRotation(forward, upwards) ; 

            //位置
            gameMesh.transform.position = new Vector3(mat.m03, mat.m13, mat.m23);
            double* u;//u value list
            double* v;//v value list
            int uv_num = 0;//u v list length
            SkpInterface.SkpDLL.GetSkpFaceUV(_skp_exporter, nGroupId, true, out u, out v, out uv_num);
            Debug.Log("UV数量 ：" + uv_num);
            float[] uarray = new float[uv_num];
            float[] varray = new float[uv_num];
            for (int i = 0; i < uv_num; i++)
            {
                uarray[i] = (float)u[i];
                varray[i] = (float)v[i];
            }

            int mat_num = SkpInterface.SkpDLL.GetMaterialNum(_skp_exporter);
            Debug.Log("材质数量 ：" + mat_num);

            int* front_mat_id;
            int* back_mat_id;
            int materialid_num = 0;

            SkpInterface.SkpDLL.GetSkpMaterialIDPerFace(_skp_exporter, nGroupId, out front_mat_id, out back_mat_id, out materialid_num);
            Debug.Log("有材质面数量 ：" + materialid_num);



            int[] materialfacecount_front = new int[mat_num];
            //liuliang
            // int[] materialfacecount_frontTest = new int[mat_num];
            //liuliang
            int[] materialfacecount_back = new int[mat_num];
            int[][] triangleindexarray = new int[mat_num][];

            for (int m = 0; m < mat_num; m++)
            {
                materialfacecount_front[m] = 0;
                materialfacecount_back[m] = 0;
            }
             for (int m = 0; m < mat_num; m++)
            {
                materialfacecount_front[m] = 0;
                materialfacecount_back[m] = 0;
            }
            int FaceMaterialIDCount = 0;
            //这里只取了正面
            for (int m = 0; m < materialid_num; m++)
            {
                int materailIndex = front_mat_id[m];
                if (materailIndex >= 0)
                {
                    materialfacecount_front[materailIndex]++;
                    FaceMaterialIDCount++;
                }
            }
            Debug.Log("正面有材质的面数量 ：" + FaceMaterialIDCount);

            FaceMaterialIDCount = 0;
            //这里只取了反面
            for (int m = 0; m < materialid_num; m++)
            {
                int materailIndex = back_mat_id[m];
                if (materailIndex >= 0)
                {
                    materialfacecount_back[materailIndex]++;
                    FaceMaterialIDCount++;
                }
            }

            Debug.Log("反面有材质的面数量 ：" + FaceMaterialIDCount);
            //设置子模型数目
            int subMeshCount = 0;
            int facetotal = 0;

            for (int m = 0; m < mat_num; m++)
            {
                int facecount = materialfacecount_front[m];
                if (facecount > 0)
                {
                    subMeshCount++;
                    triangleindexarray[m] = new int[facecount];

                    int faceindex = 0;
                    //这里只取了正面，反面没取
                    for (int n = 0; n < materialid_num; n++)
                    {
                        int materailIndex = front_mat_id[n];
                        if (materailIndex == m)
                        {
                            triangleindexarray[m][faceindex++] = n;
                            facetotal++;

                            Debug.Log("面" + n + "有材质索引 ：" + materailIndex);
                        }
                    }
                    //每个子模型最多2万面
                    subMeshCount += facecount / 20000;
                }
            }
            Debug.Log("子模型数量 ：" + subMeshCount);
            Debug.Log("面数数量 ：" + facetotal);
            Debug.Log("--2.进行材质创建----------------------------------------------------------------------------------------//");

            int TempIndex = 0;
            int[][] triangleindexarray2 = new int[subMeshCount][];
            for (int m = 0; m < mat_num; m++)
            {
                int facecount = materialfacecount_front[m];
                if (facecount > 0)
                {
                    int nLength = triangleindexarray[m].Length;

                    //新增的子模型
                    int nAddSubMesh = facecount / 20000;
                    //
                    if (0 == nAddSubMesh)
                    {
                        Debug.Log("子模型" + TempIndex + "有" + nLength + "个面");
                        triangleindexarray2[TempIndex] = new int[nLength];
                        for (int n = 0; n < nLength; n++)
                        {
                            triangleindexarray2[TempIndex][n] = triangleindexarray[m][n];

                            Debug.Log("面" + n + "的原本面索引 ：" + triangleindexarray2[TempIndex][n]);
                        }
                        TempIndex++;
                    }
                    else
                    {
                        for (int s = 0; s < nAddSubMesh; s++)
                        {
                            Debug.Log("子模型" + TempIndex + "有20000个面");
                            triangleindexarray2[TempIndex] = new int[20000];
                            for (int n = 0; n < 20000; n++)
                            {
                                triangleindexarray2[TempIndex][n] = triangleindexarray[m][n + s * 20000];

                                Debug.Log("面" + n + "的原本面索引 ：" + triangleindexarray2[TempIndex][n]);
                            }
                            TempIndex++;
                        }

                        nLength = nLength - 20000 * nAddSubMesh;
                        Debug.Log("子模型" + TempIndex + "有" + nLength + "个面");
                        triangleindexarray2[TempIndex] = new int[nLength];
                        for (int n = 0; n < nLength; n++)
                        {
                            triangleindexarray2[TempIndex][n] = triangleindexarray[m][n + nAddSubMesh * 20000];

                            Debug.Log("面" + n + "的原本面索引 ：" + triangleindexarray2[TempIndex][n]);
                        }
                        TempIndex++;
                    }
                }
            }

            Vector2[] texscaleArray = new Vector2[subMeshCount];
            gameMesh.GetComponent<MeshRenderer>().materials = new Material[subMeshCount];
            TempIndex = 0;
            for (int m = 0; m < mat_num; m++)
            // for(int m = mat_num-1; m > 0; m--)
            {
                //liuliang
                //for (int n = 0; n < materialid_num; n++)
                // {
                //       int materailIndex = front_mat_id[n];
                //       if (materailIndex != m)
                //       {
                //           continue;
                //       }
                //}
                //Debug.Log("材质ID" + m);
                //liuliang
                int facecount = materialfacecount_front[m];
                if (facecount > 0)
                {
                    Debug.Log("liuliang");
                    int nLength = triangleindexarray[m].Length;
                    //liuliang
                    for (int n = 0; n < materialid_num; n++)
                    {

                        int materailIndex = front_mat_id[n];
                        if (materailIndex == m)
                        {
                            Debug.Log(m + "liuliang面" + n + "有材质索引 ：" + materailIndex);
                        }
                    }
                    //liuliang


                    //新增的子模型
                    int nAddSubMesh = facecount / 20000;
                    //
                    if (0 == nAddSubMesh)
                    {

                        gameMesh.GetComponent<MeshRenderer>().materials[TempIndex] = meshUsematerial;

                        StringBuilder mat_name = new StringBuilder(100);
                        SkpInterface.SkpDLL.GetMaterialNameByID(_skp_exporter, m, mat_name);

                        Debug.Log("创建材质" + TempIndex + ":" + mat_name.ToString());
                        string MaterialName = mat_name.ToString();
                        gameMesh.GetComponent<MeshRenderer>().materials[TempIndex].name = MaterialName;

                        bool has_color = false;
                        double[] color = new double[3];
                        bool has_alpha = false;
                        double alpha = 1;
                        bool has_texture = false;
                        int bits_per_pixel = 0;
                        int data_size = 0;
                        int width = 0, height = 0;
                        double* pixel_data;
                        int groupid = -1;
                        StringBuilder tex_p = new StringBuilder();
                        SkpInterface.SkpDLL.GetSkpMaterialData(_skp_exporter,
                                groupid,
                                 m,
                                 out has_color,
                                 color,
                                 out has_alpha,
                                 out alpha,
                                 out has_texture,
                                 out bits_per_pixel,
                                 out data_size,
                                 out width,
                                 out height,
                                 out pixel_data
                                 );

                        Color newColor = new Color(1, 1, 1, 1);

                        //Debug.Log("纹理缩放X -------" + tex_sscale);
                        // Debug.Log("纹理缩放T -------" + tex_tscale);

                        //texscaleArray[TempIndex] = new Vector2((float)tex_sscale, (float)tex_tscale);
                        texscaleArray[TempIndex] = new Vector2((float)1.0, (float)1.0);
                        if (has_color)
                        {
                            //Debug.Log("Color :" + color[0] + " " + color[1] + " " + color[2]);
                            //this.gameObject.GetComponent<MeshRenderer>().materials[SubmeshMaterialIndex].color = new Color((float)color[0], (float)color[1], (float)color[2]);
                            newColor.r = (float)color[0];
                            newColor.g = (float)color[1];
                            newColor.b = (float)color[2];

                        }

                        if (has_alpha)
                        {
                            //Debug.Log("Alpha :" + alpha);
                            newColor.a = (float)alpha;
                        }

                        gameMesh.GetComponent<MeshRenderer>().materials[TempIndex].color = newColor;
                        gameMesh.GetComponent<MeshRenderer>().materials[TempIndex].SetColor("Albedo", newColor);

                        if (has_texture)
                        {
                            //如果纹理存在，应该打印出纹理路径
                            // Debug.Log("Texture :" + tex_path.ToString());
                            // string texturePath = "file:///" + tSingleMeshInfo.meshData.FilePath + "/" + tex_path.ToString();
                            // StartCoroutine(IGetSKPTexture(texturePath, TempIndex));
                            // Texture2D texture = new Texture2D(width, height, TextureFormat.ARGB32, false);
                            // texture.
                            //  texture.Apply();

                            //Texture2D textrue = Resources.Load("materialBox/Roofing_Slate_Dark") as Texture2D;



                            Texture2D textrue1 = new Texture2D(width, height, TextureFormat.ARGB32, true);
                            if (bits_per_pixel == 24)
                            {
                                Color[] color1 = new Color[data_size / 3];
                                for (int i = 0; i < color1.Length; i++)
                                {
                                    color1[i] = new Color((float)pixel_data[i * 3], (float)pixel_data[i * 3 + 1], (float)pixel_data[i * 3 + 2]);
                                }
                                textrue1.SetPixels(color1);
                            }
                            else if (bits_per_pixel == 32)
                            {
                                Color[] color1 = new Color[data_size / 4];
                                for (int i = 0; i < color1.Length; i++)
                                {
                                    color1[i] = new Color((float)pixel_data[i * 4], (float)pixel_data[i * 4 + 1], (float)pixel_data[i * 4 + 2]);
                                }
                                textrue1.SetPixels(color1);
                            }



                            textrue1.Apply();
                            gameMesh.GetComponent<MeshRenderer>().materials[TempIndex].mainTexture = textrue1;
                        }

                        TempIndex++;
                    }
                    else
                    {
                        for (int s = 0; s <= nAddSubMesh; s++)
                        {
                            gameMesh.GetComponent<MeshRenderer>().materials[TempIndex] = meshUsematerial;

                            StringBuilder mat_name = new StringBuilder(100);
                            SkpInterface.SkpDLL.GetMaterialNameByID(_skp_exporter, m, mat_name);

                            Debug.Log("创建材质" + TempIndex + ":" + mat_name.ToString());
                            string MaterialName = mat_name.ToString();

                            bool has_color = false;
                            double[] color = new double[3];
                            bool has_alpha = false;
                            double alpha = 1;
                            bool has_texture = false;
                            int bits_per_pixel = 0;
                            int data_size = 0;
                            int width = 0, height = 0;
                            double* pixel_data;
                            int groupid = -1;
                            StringBuilder tex_p = new StringBuilder();
                            SkpInterface.SkpDLL.GetSkpMaterialData(_skp_exporter,
                                    groupid,
                                     m,
                                     out has_color,
                                     color,
                                     out has_alpha,
                                     out alpha,
                                     out has_texture,
                                     out bits_per_pixel,
                                     out data_size,
                                     out width,
                                     out height,
                                     out pixel_data
                                     );

                            Color newColor = new Color(1, 1, 1, 1);

                            //Debug.Log("纹理缩放X -------" + tex_sscale);
                            // Debug.Log("纹理缩放T -------" + tex_tscale);

                            //texscaleArray[TempIndex] = new Vector2((float)tex_sscale, (float)tex_tscale);
                            texscaleArray[TempIndex] = new Vector2((float)1.0, (float)1.0);
                            if (has_color)
                            {
                                //Debug.Log("Color :" + color[0] + " " + color[1] + " " + color[2]);
                                //this.gameObject.GetComponent<MeshRenderer>().materials[SubmeshMaterialIndex].color = new Color((float)color[0], (float)color[1], (float)color[2]);
                                newColor.r = (float)color[0];
                                newColor.g = (float)color[1];
                                newColor.b = (float)color[2];

                            }

                            if (has_alpha)
                            {
                                //Debug.Log("Alpha :" + alpha);
                                newColor.a = (float)alpha;
                            }

                            gameMesh.GetComponent<MeshRenderer>().materials[TempIndex].color = newColor;
                            gameMesh.GetComponent<MeshRenderer>().materials[TempIndex].SetColor("Albedo", newColor);

                            if (has_texture)
                            {

                                Texture2D textrue1 = new Texture2D(width, height, TextureFormat.ARGB32, true);
                                if (bits_per_pixel == 24)
                                {
                                    Color[] color1 = new Color[data_size / 3];
                                    for (int i = 0; i < color1.Length; i++)
                                    {
                                        color1[i] = new Color((float)pixel_data[i * 3], (float)pixel_data[i * 3 + 1], (float)pixel_data[i * 3 + 2]);
                                    }
                                    textrue1.SetPixels(color1);
                                }
                                else if (bits_per_pixel == 32)
                                {
                                    Color[] color1 = new Color[data_size / 4];
                                    for (int i = 0; i < color1.Length; i++)
                                    {
                                        color1[i] = new Color((float)pixel_data[i * 4], (float)pixel_data[i * 4 + 1], (float)pixel_data[i * 4 + 2]);
                                    }
                                    textrue1.SetPixels(color1);
                                }



                                textrue1.Apply();
                            }

                            TempIndex++;
                        }
                    }

                }
            }




            Debug.Log("--3.进行模型创建----------------------------------------------------------------------------------------//");

            double* _vertices;
            int _vertex_num;
            int* _vertex_num_per_face;
            int _face_num;
            double* _face_normal;
            SkpInterface.SkpDLL.GetSkpFace(_skp_exporter, nGroupId, out _vertices, out _vertex_num, out _vertex_num_per_face, out _face_num, out _face_normal);
            Debug.Log("面数 ：" + _face_num);

            int[] FaceVertexIndex = new int[_face_num * 3];

            for (int f = 0; f < _face_num * 3; f++)
            {
                FaceVertexIndex[f] = f;
            }

            //通过查询FaceVertexIndex发现92个面的顶点索引总数也为92,而不是276


            //------------------------------------------------------------------------------------------//
            int nRealVertexNum = 0;
            for (int s = 0; s < subMeshCount; s++)
            {
                nRealVertexNum += triangleindexarray2[s].Length * 3;
            }

            Debug.Log("=====================重新计算顶点数:" + nRealVertexNum + "==========================");


            Vector3[] PositionArray = new Vector3[nRealVertexNum];
            Vector3[] NormalArray = new Vector3[nRealVertexNum];
            Vector2[] UVArray = new Vector2[nRealVertexNum];
            int[] IndexArray = new int[nRealVertexNum];
            int[] SubMeshVertexIndexEnd = new int[subMeshCount];
            int nRealVertexIndex = 0;

            for (int s = 0; s < subMeshCount; s++)
            {
                int nSubMeshFaceCount = triangleindexarray2[s].Length;
                Debug.Log("================第" + s + "个子模型面数:" + nSubMeshFaceCount + "==========================");

                for (int f = 0; f < nSubMeshFaceCount; f++)
                {
                    int Index_Old = triangleindexarray2[s][f];
                    //Debug.Log("面" + f + "的原本面索引 ：" + triangleindexarray2[s][f]);


                    int nRealFaceIndex1 = FaceVertexIndex[Index_Old * 3];
                    int nRealFaceIndex2 = FaceVertexIndex[Index_Old * 3 + 1];
                    int nRealFaceIndex3 = FaceVertexIndex[Index_Old * 3 + 2];

                    IndexArray[nRealVertexIndex] = nRealVertexIndex;
                    Debug.Log("第" + f + "个面索引:" + "V1Index:" + nRealFaceIndex1 + "  V2Index:" + nRealFaceIndex2 + "  V3Index:" + nRealFaceIndex3);
                    //==============================V1===========================
                    int Index_1 = nRealFaceIndex1 * 3;
                    int Index_2 = nRealFaceIndex1 * 3 + 1;
                    int Index_3 = nRealFaceIndex1 * 3 + 2;

                    PositionArray[nRealVertexIndex] = new Vector3((float)_vertices[Index_1], (float)_vertices[Index_2], (float)_vertices[Index_3]);
                    NormalArray[nRealVertexIndex] = new Vector3((float)_vertices[Index_1], (float)_vertices[Index_2], (float)_vertices[Index_3]);

                    UVArray[nRealVertexIndex] = new Vector2((float)uarray[nRealFaceIndex1] * texscaleArray[s].x, (float)varray[nRealFaceIndex1] * texscaleArray[s].y);
                    nRealVertexIndex++;

                    //==============================V2===========================
                    Index_1 = nRealFaceIndex2 * 3;
                    Index_2 = nRealFaceIndex2 * 3 + 1;
                    Index_3 = nRealFaceIndex2 * 3 + 2;

                    IndexArray[nRealVertexIndex] = nRealVertexIndex;
                    PositionArray[nRealVertexIndex] = new Vector3((float)_vertices[Index_1], (float)_vertices[Index_2], (float)_vertices[Index_3]);
                    NormalArray[nRealVertexIndex] = new Vector3((float)_vertices[Index_1], (float)_vertices[Index_2], (float)_vertices[Index_3]);

                    UVArray[nRealVertexIndex] = new Vector2((float)uarray[nRealFaceIndex2] * texscaleArray[s].x, (float)varray[nRealFaceIndex2] * texscaleArray[s].y);

                    nRealVertexIndex++;

                    //==============================V3===========================
                    Index_1 = nRealFaceIndex3 * 3;
                    Index_2 = nRealFaceIndex3 * 3 + 1;
                    Index_3 = nRealFaceIndex3 * 3 + 2;

                    IndexArray[nRealVertexIndex] = nRealVertexIndex;
                    PositionArray[nRealVertexIndex] = new Vector3((float)_vertices[Index_1], (float)_vertices[Index_2], (float)_vertices[Index_3]);
                    NormalArray[nRealVertexIndex] = new Vector3((float)_vertices[Index_1], (float)_vertices[Index_2], (float)_vertices[Index_3]);

                    UVArray[nRealVertexIndex] = new Vector2((float)uarray[nRealFaceIndex3] * texscaleArray[s].x, (float)varray[nRealFaceIndex3] * texscaleArray[s].y);

                    nRealVertexIndex++;
                }

                SubMeshVertexIndexEnd[s] = nRealVertexIndex;
            }

            Debug.Log("--4.进行子模型创建----------------------------------------------------------------------------------------//");

            mesh.vertices = PositionArray;
            mesh.normals = NormalArray;
            mesh.triangles = IndexArray;
            mesh.uv = UVArray;
            mesh.RecalculateNormals();
            mesh.RecalculateBounds();
            mesh.subMeshCount = subMeshCount;

            for (int s = 0; s < subMeshCount; s++)
            {
                if (0 == s)
                {
                    mesh.SetTriangles(GetRangeArray(IndexArray, 0, SubMeshVertexIndexEnd[s] - 1), s, true);
                    Debug.Log("SubMesh" + s + "V Start:" + 0 + " V End:" + SubMeshVertexIndexEnd[s]);
                }
                else
                {
                    mesh.SetTriangles(GetRangeArray(IndexArray, SubMeshVertexIndexEnd[s - 1], SubMeshVertexIndexEnd[s] - 1), s,true);
                    Debug.Log("SubMesh" + s + "V Start:" + SubMeshVertexIndexEnd[s - 1] + " V End:" + SubMeshVertexIndexEnd[s]);
                }

            }

            int* children_id;
            int children_num;
            SkpInterface.SkpDLL.GetSkpGroupChildrenById(_skp_exporter, nGroupId, out children_id, out children_num);
            for (int j = 0; j < children_num; j++)
            {
                 creatGameMesh(children_id[j], _skp_exporter, gameMesh);
            }
        }
    }
}
