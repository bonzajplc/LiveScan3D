using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Net.Sockets;
using System.Runtime.Serialization.Formatters.Binary;

namespace KinectServer
{
    public class TransferSocket
    {
        TcpClient oSocket;

        public TransferSocket(TcpClient clientSocket)
        {
            oSocket = clientSocket;
        }

        public byte[] Receive(int nBytes)
        {
            byte[] buffer;
            if (oSocket.Available != 0)
            {
                buffer = new byte[Math.Min(nBytes, oSocket.Available)];
                oSocket.GetStream().Read(buffer, 0, nBytes);
            }
            else
                buffer = new byte[0];

            return buffer;
        }

        public bool SocketConnected()
        {
            return oSocket.Connected;
        }

        public void WriteInt(int val)
        {
            oSocket.GetStream().Write(BitConverter.GetBytes(val), 0, 4);
        }

        public void WriteFloat(float val)
        {
            oSocket.GetStream().Write(BitConverter.GetBytes(val), 0, 4);
        }

        public void SendFrame(List<float> vertices, List<short> normals, List<short> uvs, List<byte> colors, List<ushort> indices)
        {
            short[] sVertices = Array.ConvertAll(vertices.ToArray(), x => (short)(x * 1000));
            short[] sNormals = normals.ToArray();
            short[] sUVs = uvs.ToArray();

            int nVerticesToSend = vertices.Count / 3;
            byte[] buffer = new byte[sizeof(short) * ( 3 + 2 + 2 ) * nVerticesToSend];
            Buffer.BlockCopy(sVertices, 0, buffer, 0, sizeof(short) * 3 * nVerticesToSend);
            Buffer.BlockCopy(sNormals, 0, buffer, sizeof(short) * 3 * nVerticesToSend, sizeof(short) * 3 * nVerticesToSend);
            Buffer.BlockCopy(sUVs, 0, buffer, sizeof(short) * ( 3 + 3 ) * nVerticesToSend, sizeof(short) * 2 * nVerticesToSend);

            int nIndicesToSend = indices.Count;
            byte[] indexBuffer = new byte[sizeof(short) * nIndicesToSend];
            Buffer.BlockCopy(indices.ToArray(), 0, indexBuffer, 0, sizeof(short) * nIndicesToSend);
            try
            {                 
                WriteInt(nVerticesToSend);                               
                oSocket.GetStream().Write(buffer, 0, buffer.Length);
                oSocket.GetStream().Write(colors.ToArray(), 0, sizeof(byte) * 4 * nVerticesToSend);
                WriteInt(nIndicesToSend);
                oSocket.GetStream().Write(indexBuffer, 0, indexBuffer.Length);
            }
            catch (Exception ex)
            {
            }
        }
    }
}
