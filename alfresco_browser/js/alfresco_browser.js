// $Id$

//
// Main layout definition.
//
Ext.onReady(function() {

  //Ext.state.Manager.setProvider(new Ext.state.CookieProvider());

  Ext.QuickTips.init();
  
  var opener = window.opener;
  
  var treePanel = new Ext.tree.TreePanel({
    id            : 'tree-panel',
    title         : 'Browse Spaces',
    region        : 'north',
    split         : true,
    height        : 300,
    minSize       : 150,
    autoScroll    : true,

    // tree-specific configs:
    rootVisible   : false,
    lines         : false,
    singleExpand  : false,
    trackMouseOver: false,
    useArrows     : true,

    loader : new Ext.tree.TreeLoader({
      dataUrl: Drupal.settings.alfresco_browser.urlSpaces
    }),

    root: new Ext.tree.AsyncTreeNode({id: 'null'}),
    
    tools:[{
      id: 'refresh',
      on: {
        click: function(){
          var tree = Ext.getCmp('tree-panel');
          tree.body.mask('Loading', 'x-mask-loading');
          tree.root.reload();
          setTimeout(function() {
            tree.body.unmask();
          }, 1000);
          store.load({params:{node:'null'}});
        }
      }
    }],
    
    listeners: {
      'click': function(node, e){
        node.expand();
        store.load({params:{node:node.id}});
      }
    }
  });

  // Reader
  var reader = new Ext.data.JsonReader({
    root: 'nodes'
  }, [
     {name: 'uuid'},
     {name: 'type'},
     {name: 'name', 'type': 'string', 'sortType': 'asUCString'},
     {name: 'title', 'type': 'string', 'sortType': 'asUCString'},
     {name: 'size', 'type': 'string'},
     {name: 'exists'},
     {name: 'created', type: 'date', dateFormat: 'Y-m-d H:i:s'},
     {name: 'modified', type: 'date', dateFormat: 'Y-m-d H:i:s'}
  ]);

  // Data Store
  var store = new Ext.data.GroupingStore({
    url: Drupal.settings.alfresco_browser.urlItems,
    reader: reader,
    remoteSort: false,
    sortInfo: {field: 'name', direction: 'ASC'},
    groupField: 'type'
  });
  
  // Content Items Grid
  var grid = new Ext.grid.GridPanel( {
    id: 'content-items',
    store: store,
    columns: [ {
      id: 'uuid',
      header: "UUID",
      sortable: true,
      dataIndex: 'uuid',
      hidden: true
    }, {
      id: 'type',
      header: "Type",
      sortable: true,
      dataIndex: 'type'
    }, {
      id: 'name',
      header: "Name",
      sortable: true,
      dataIndex: 'name'
    }, {
      header: "Title",
      sortable: true,
      dataIndex: 'title',
      hidden: true
    }, {
      header: "Size",
      width: 75,
      sortable: false,
      dataIndex: 'size'
    }, {
      header: "Created",
      width: 125,
      sortable: true,
      renderer: Ext.util.Format.dateRenderer('Y-m-d H:i:s'),
      dataIndex: 'created'
    }, {
      header: "Modified",
      width: 125,
      sortable: true,
      renderer: Ext.util.Format.dateRenderer('Y-m-d H:i:s'),
      dataIndex: 'modified'
    }],
    
    listeners: {
      'rowdblclick': function(grid, rowIndex, e){
        
        var row = this.store.getAt(rowIndex);
        var uuid = row.get('uuid');
        
        if (row.get('type') == 0 && opener.$("form#alfresco-import-form").length == 0) {
          store.load({params:{node:uuid}});

          var node = treePanel.getNodeById(uuid);
          if (node) {
            node.expand();
            node.select();
  
            var ancestors = [];
            var i = 0;
            while (node.parentNode) {
               ancestors[i] = node.parentNode;
               i = i + 1;
               node = node.parentNode;
            }
            for (i = (ancestors.length - 1); i >= 0; i--) {
               ancestors[i].expand(false, false);
            }
          }
        }
        else {
          var nodeRef = 'workspace://SpacesStore/' + uuid;
          var title = row.get('title');
          opener.$("#edit-alfresco-browser-reference").val(nodeRef);
          opener.$("#alfresco-edit-title-wrapper #edit-title").val(title);
          opener.focus();
          self.close();
        }
      }
    },
    
    view: new Ext.grid.GroupingView({
      enableGroupingMenu : false,
      hideGroupedColumn : true,
      groupTextTpl : '{[values.gvalue == "0" ? "Spaces" : "Items"]} ({[values.rs.length]})',
      getRowClass : function(record, index) {
        var exists = record.get('exists') == '1' ? 'node-exists' : 'node-new';
        var type   = record.get('type');
        return 'icon-' + type + ' ' + exists;
      }
    }),

    // inline toolbars
    /*
    tbar:[{
      text:'Add Something',
      tooltip:'Add a new row',
      iconCls:'add'
    }, '-', {
      text:'Options',
      tooltip:'Blah blah blah blaht',
      iconCls:'option'
    },'-',{
      text:'Remove Something',
      tooltip:'Remove the selected item',
      iconCls:'remove'
    }],
    */

    autoExpandColumn : 'name',
    trackMouseOver   : false,
    loadMask         : true
  });

  new Ext.Viewport({
    layout : 'border',
    items  : [{
      region      : 'north',
      height      : 35,
      html        : '<div id="header"><h1>Alfresco Node Browser for Drupal</h1></div>'
    }, {
      id          : 'layout-browser',
      layout      : 'fit',
      region      : 'west',
      border      : false,
      split       : true,
      margins     : '2 0 5 5',
      width       : 200,
      minSize     : 100,
      maxSize     : 500,
      items       : treePanel
    }, {
      id          : 'content-panel',
      title       : 'Content Items',
      region      : 'center',
      layout      : 'fit',
      margins     : '2 5 5 0',
      border      : false,
      items       : grid
    }/*, {
      region      : 'south',
      buttons     : [{text:'Close'}],
      buttonAlign : 'center'
    }*/],
    renderTo : Ext.getBody()
  });
  
  // trigger the data store load
  store.load({params:{node:'null'}});
});
