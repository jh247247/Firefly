using rediscover.DataModel;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using Windows.Foundation;
using Windows.Foundation.Collections;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Controls.Primitives;
using Windows.UI.Xaml.Data;
using Windows.UI.Xaml.Input;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

// The User Control item template is documented at http://go.microsoft.com/fwlink/?LinkId=234236

namespace rediscover
{
    public sealed partial class ItemViewerFirefly : UserControl
    {
        public ItemViewerFirefly()
        {
            this.InitializeComponent();
        }

        public void ShowPlaceholder(Firefly item)
        {
            _firefly = item;
            txtFireflyId.Opacity = 0;
            txtFireflyAttribute.Opacity = 0;
        }

        public void ShowId()
        {
            txtFireflyId.Text = _firefly.Id;
            txtFireflyId.Opacity = 1;
        }

        public void ShowAttribute()
        {
            txtFireflyAttribute.Text = _firefly.Attribute;
            txtFireflyAttribute.Opacity = 1;
        }

        public void ClearData()
        {
            _firefly = null;
            txtFireflyId.ClearValue(TextBlock.TextProperty);
            txtFireflyAttribute.ClearValue(TextBlock.TextProperty);
        }

        private Firefly _firefly;
    }
}
