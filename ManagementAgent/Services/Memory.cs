using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.MemoryMappedFiles;
using System.IO;
using System.ComponentModel;
using System.Threading;

namespace ManagementAgent.Services
{
    class Memory
    {
        public static int Connected { get; set; }
        public static int CLients { get; set; }
        public static UInt32 DataSent { get; set; }
        public static UInt32 DataRec { get; set; }

        public static void init()
        {

            MemoryMappedFile MemoryMapped = MemoryMappedFile.CreateFromFile(
                                @"Mem.mp",                                          // Any stream will do it
                                FileMode.Create,
                                "CloudAgent",                                     // Name
                                1024,                                        // Size in bytes
                                MemoryMappedFileAccess.ReadWrite);                  // Access type

            MemoryMappedViewAccessor FileMapView = MemoryMapped.CreateViewAccessor();

            int Number = 1234;
            
            FileMapView.Write(0, Number);
            //FileMapView.Write<Container>(4, ref MyContainer);

        }

        public static string OpenMem() {

            try
            {
                // Assumes another process has created the memory-mapped file.
                using (var mmf = MemoryMappedFile.OpenExisting("CloudAgent"))
                {
                    using (var accessor = mmf.CreateViewAccessor(0, 1024, MemoryMappedFileAccess.Read))
                    {
                        //StringBuilder outl = new StringBuilder("Connected ").Append(accessor.ReadByte(0).ToString()).Append("  :  ")
                        //  .Append("Clients = ").Append(accessor.ReadByte(1).ToString());

                        Connected = accessor.ReadByte(0);
                        CLients = accessor.ReadByte(1);
                        DataSent = accessor.ReadUInt32(2);
                        DataRec = accessor.ReadUInt32(6);
                        return "";
                        //for (int i = 1; i < 1022; i++)
                        //{
                        //    outl.Append(accessor.ReadChar(i));
                        //    i++;
                        //}
                        //return outl.ToString();
                        //int colorSize = Marshal.SizeOf(typeof(MyColor));
                        //MyColor color;

                        // Make changes to the view.
                        //for (long i = 0; i < 1500000; i += colorSize)
                        //{
                        //accessor.Read(i, out color);
                        //color.Brighten(20);
                        //accessor.Write(i, ref color);
                        //}
                    }
                }
            }
            catch (Exception ex)
            {
                return ("Memory-mapped file does not exist. Run Process A first.");
            }
            return "";
        }

    }
}
