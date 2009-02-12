// $Id$

if (Drupal.jsEnabled) {

  Ext.onReady( function() {

    var button = Ext.get('edit-alfresco-browser-button');

    button.on('click', function() {

      var panel = new Ext.Panel({
        region :'center'
      });

      // tabs for the center
        var tabs = new Ext.TabPanel( {
          region :'center',
          margins :'3 3 3 0',
          activeTab :0,
          defaults : {
            autoScroll :true
          },
          items : [ {
            title :'Bogus Tab',
            html :''
          }, {
            title :'Another Tab',
            html :''
          }, {
            title :'Closable Tab',
            html :'',
            closable :true
          } ]
        });

        // Panel for the west
        var nav = new Ext.Panel( {
          title :'Navigation',
          region :'west',
          split :true,
          width :200,
          collapsible :true,
          margins :'3 0 3 3',
          cmargins :'3 3 3 3'
        });

        var win = new Ext.Window( {
          title :'Layout Window',
          closable :true,
          modal :true,
          width :600,
          height :350,
          // border : false,
          plain :true,
          layout :'border',
          items : [ nav, panel ]
        });

        win.show(button);
      });
  });
}
