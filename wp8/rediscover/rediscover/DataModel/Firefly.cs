using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace rediscover.DataModel
{
    public class Firefly : System.ComponentModel.INotifyPropertyChanged
    {
        public event System.ComponentModel.PropertyChangedEventHandler PropertyChanged;

        public Firefly(string initId)
        {
            Id = initId;
        }

        public Firefly(string initId, string initAttribute)
        {
            Id = initId;
            Attribute = initAttribute;
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

        private string _Battery = string.Empty;
        public string Battery
        {
            get
            {
                return this._Battery;
            }

            set
            {
                if (this._Battery != value)
                {
                    this._Battery = value;
                    this.OnPropertyChanged("Battery");
                }
            }
        }

        private string _LastUpdateTime = string.Empty;
        public string LastUpdateTime
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

        private string _NodeId = string.Empty;
        public string NodeId
        {
            get
            {
                return this._NodeId;
            }

            set
            {
                if (this._NodeId != value)
                {
                    this._NodeId = value;
                    this.OnPropertyChanged("NodeId");
                }
            }
        }

        private string _Attribute = string.Empty;
        public string Attribute
        {
            get
            {
                return this._Attribute;
            }

            set
            {
                if (this._Attribute != value)
                {
                    this._Attribute = value;
                    this.OnPropertyChanged("Attribute");
                }
            }
        }
    }

    public class FireflyCollection : IEnumerable<Object>
    {
        private System.Collections.ObjectModel.ObservableCollection<Firefly> fireflyCollection = new System.Collections.ObjectModel.ObservableCollection<Firefly>();

        public IEnumerator<Object> GetEnumerator()
        {
            return fireflyCollection.GetEnumerator();
        }

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }

        public void Add(Firefly firefly)
        {
            fireflyCollection.Add(firefly);
        }

        public void Add(Object firefly)
        {
            fireflyCollection.Add((Firefly) firefly);
        }

        public void Remove(Firefly firefly)
        {
            fireflyCollection.Remove(firefly);
        }

        public bool Contains(Firefly firefly)
        {
            foreach (Firefly fireflyTocheck in fireflyCollection)
            {
                if (fireflyTocheck.Id == firefly.Id)
                {
                    return true;
                }
            }
            return false;
        }
    }
}
