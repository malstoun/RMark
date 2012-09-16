using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Limin
{
    class vars
    {
        protected vars()
        { }

        private sealed class varsCreate
        {
            private static readonly vars instance = new vars();
            public static vars Instance
            {
                get
                {
                    return instance;
                }
            }
        }

        public static vars VARS
        {
            get
            {
                return varsCreate.Instance;
            }
        }

        string serverExeFile = "Limserver.exe";
        string serverExeParams = "";

        public string ServerExeFile
        {
            get
            {
                return serverExeFile;
            }
            set
            {
                serverExeFile = value;
            }
        }

        public string ServerExeParams
        {
            get
            {
                return serverExeParams;
            }
            set
            {
                serverExeParams = value;
            }
        }
    }
}
