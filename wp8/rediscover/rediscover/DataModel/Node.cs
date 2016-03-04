using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace rediscover.DataModel
{
    public class Node : System.ComponentModel.INotifyPropertyChanged
    {
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;

        public Node()
        {
        }

        public Node(string initId)
        {
            Id = initId;
        }

        public Node(string initId, string initLocation)
        {
            Id = initId;
            Location = initLocation;
        }

        protected virtual void OnPropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                this.PropertyChanged(this, new System.ComponentModel.PropertyChangedEventArgs(propertyName));
            }
        }

        private string _Id = string.Empty;
        public string Id
        {
            get
            {
                return this._Id;
            }

            set
            {
                if (this._Id != value)
                {
                    this._Id = value;
                    this.OnPropertyChanged("Id");
                }
            }
        }

        private DateTime _LastUpdateTime = new DateTime(0);
        public DateTime LastUpdateTime
        {
            get
            {
                return this._LastUpdateTime;
            }

            set
            {
                if (this._LastUpdateTime != value)
                {
                    this._LastUpdateTime = value;
                    this.OnPropertyChanged("LastUpdateTime");
                }
            }
        }

        private string _FirefliesList = string.Empty;
        public string FirefliesList
        {
            get
            {
                return this._FirefliesList;
            }

            set
            {
                if (this._FirefliesList != value)
                {
                    this._FirefliesList = value;
                    this.OnPropertyChanged("FirefliesList");
                }
            }
        }

        private string _Location = string.Empty;
        public string Location
        {
            get
            {
                return this._Location;
            }

            set
            {
                if (this._Location != value)
                {
                    this._Location = value;
                    this.OnPropertyChanged("Location");
                }
            }
        }
    }

    public class NodeCollection : IEnumerable<Object>
    {
        private System.Collections.ObjectModel.ObservableCollection<Node> nodeCollection = new System.Collections.ObjectModel.ObservableCollection<Node>();

        public IEnumerator<Object> GetEnumerator()
        {
            return nodeCollection.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public void Add(Node node)
        {
            nodeCollection.Add(node);
        }

        public void Add(Object node)
        {
            nodeCollection.Add((Node)node);
        }

        public void Remove(Node node)
        {
            nodeCollection.Remove(node);
        }

        public void Clear()
        {
            nodeCollection.Clear();
        }

        public bool Contains(Node node)
        {
            foreach (Node nodeTocheck in nodeCollection)
            {
                if (nodeTocheck.Id == node.Id)
                {
                    return true;
                }
            }
            return false;
        }
    }
}
