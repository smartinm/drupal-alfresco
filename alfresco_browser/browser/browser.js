// $Id$

/*
 * Ext JS Alfresco Browser for Drupal.
 * 
 * Module Pattern
 * http://extjs.com/learn/Manual:Basic_Application_Design
 */
Ext.ns('AlfrescoBrowser');

AlfrescoBrowser.ViewItem = function (itemsGrid) {
  var items = itemsGrid.getSelectionModel().getSelections();
  if (items.length == 0) {
    return;
  }

  var node = items[0].data;
  var url = AlfrescoBrowser.Settings['serviceDownloadUrl'] + "/" + node.name + "?node=" + node.id;
  var size = Ext.getCmp('alfresco-browser-viewport').getSize();
  
  var iframeWin = new Ext.Window({
    id: 'preview-window',
    title: node.name,
    width: size.width - 100,
    height: size.height - 100,
    maximizable: true,
    modal: 'true',
    layout: 'fit', 
    html: '<iframe id="preview-frame" frameborder="0"  src="' + url + '" onLoad="AlfrescoBrowser.ViewItemOnLoad()"></iframe>',
    buttonAlign: 'center',
    defaultButton: 0,
    buttons: [{
      text: 'Close',
      handler: function() {
        iframeWin.close();
      }
    }]
  });
  
  iframeWin.show();
  
  var mask = new Ext.LoadMask(iframeWin.body);
  mask.show();
}

AlfrescoBrowser.ViewItemOnLoad = function () {
  var iframeWin = Ext.getCmp('preview-window');
  if (iframeWin) {
    iframeWin.body.unmask();
  }
}

AlfrescoBrowser.SendItem = function (itemsGrid) {
  var items = itemsGrid.getSelectionModel().getSelections();
  if (items.length == 0 || !window.opener) {
    return;
  }
  
  var node = items[0].data;
  var title = Ext.util.Format.trim(node.title);
  var reference = 'workspace://SpacesStore/' + node.id;
  
  window.opener.$("#edit-alfresco-browser-reference").val(reference);
  window.opener.$("#edit-alfresco-browser-path").val(node.name);
  
  if (window.opener.$("#alfresco-edit-title-wrapper #edit-title").length > 0) {
    window.opener.$("#alfresco-edit-title-wrapper #edit-title").val(title);
  }
  
  window.opener.focus();
  self.close();
}

AlfrescoBrowser.DownloadItem = function (itemsGrid, forceDownload) {
  var items = itemsGrid.getSelectionModel().getSelections();
  if (items.length == 0) {
    return;
  }

  var node = items[0].data;
  var url = AlfrescoBrowser.Settings['serviceDownloadUrl'] + "/" + node.name + "?node=" + node.id;
  
  if (forceDownload) {
    url += "&mode=attachment";
  }
  
  if (Ext.isIE) {
    window.location = url;
  } else {
    window.open(url);
  }
}

AlfrescoBrowser.DeleteItem = function (itemsGrid) {
  var items = itemsGrid.getSelectionModel().getSelections();
  if (items.length == 0) {
    return;
  }
  
  var node = items[0].data;
  
  Ext.MessageBox.confirm('Confirm', 'Are you sure you want to delete "' + node.name + '" and all previous versions?', function(btn) {
    if (btn == 'yes') {
      var url = AlfrescoBrowser.Settings['serviceDeleteUrl'] + "/" + node.name + "?node=" + node.id;
      Ext.Ajax.request({
        url: url,
        success: function(response, options) {
          var result = Ext.decode(response.responseText);
          if (result.success) {
            itemsGrid.store.load({params:{start: 0, cache: 'node'}});
          } else {
            Ext.MessageBox.alert('Ha ocurrido un error.');
          }
        },
        failure: function() {
          Ext.MessageBox.alert('Ha ocurrido un error.');
        }
      });
    }
  });
}

AlfrescoBrowser.AddItem = function (folderTree, dataStore) {
  var space = folderTree.getSelectionModel().getSelectedNode();
  if (Ext.isEmpty(space)) {
    return;
  }

  var uploadForm = new Ext.FormPanel({
    fileUpload: true,
    width: 500,
    frame: true,
    border: false,
    autoHeight: true,
    bodyStyle: 'padding: 10px 10px 0 10px;',
    labelWidth: 50,
    defaults: {
      anchor: '95%',
      allowBlank: false,
      msgTarget: 'side'
    },
    items: [{
      xtype: 'fileuploadfield',
      id: 'form-file',
      name: 'files[file]',  // No change: required for Drupal Form API
      emptyText: 'Locate content to upload',
      fieldLabel: 'File',
      labelStyle: 'font-weight:bold;',
      listeners: {
        'fileselected': function(fb, value) {
          if (value) {
            Ext.getCmp('form-name').setValue(value);
          }
        }
      }
    }, {
      xtype: 'textfield',
      id: 'form-name',
      name: 'name',
      fieldLabel: 'Name',
      labelStyle: 'font-weight:bold;',
      maxLength: 255
    }, {
      xtype: 'hidden',
      name: 'space',
      value: space.id
    }, {
      xtype: 'fieldset',
      title: 'Content properties',
      defaultType: 'textfield',
      labelWidth: 70,
      anchor: '100%',
      style: 'margin-top: 10px;',
      defaults: {
        anchor: '95%',
        msgTarget: 'side'
      },
      autoHeight:true,
      items: [{
        fieldLabel: 'Title',
        name: 'title',
        allowBlank: false,
        labelStyle: 'font-weight:bold;',
        maxLength: 255
      },{
        xtype: 'textarea',
        fieldLabel: 'Description',
        name: 'description'
      },{
        fieldLabel: 'Author',
        name: 'author',
        maxLength: 255
      }]
    }],
    buttons: [{
      text: 'Add',
      handler: function() {
        if (uploadForm.getForm().isValid()) {
          uploadForm.getForm().submit({
            url: AlfrescoBrowser.Settings['serviceUploadUrl'],
            waitMsg: 'Uploading your content...',
            success: function(form, o) {
              dataStore.load({params:{start: 0, cache: 'node'}});
              uploadWindow.close();
            },
            failure: function(form, o){
              Ext.MessageBox.alert('Ha ocurrido un error.', o.result);
            }
          });
        }
      }
    }, {
      text: 'Cancel',
      handler: function(){
        uploadWindow.close();
      }
    }]
  });

  var uploadWindow = new Ext.Window({
    id: 'upload-window',
    title: 'Add content to current space: ' + space.text,
    autoHeight: true,
    minWidth: 500,
    modal: 'true',
    layout: 'fit', 
    items: [ uploadForm ]
  });
  
  uploadWindow.show();
}

AlfrescoBrowser.App = function() {
  var folderTree;
  var itemsGrid;
  var propsGrid;
  var dataStore;
  var currentPath;

  return {
    init: function() {
      //Ext.state.Manager.setProvider(new Ext.state.CookieProvider());
      
      Ext.QuickTips.init();
      
      this.initFolderTree();
      this.initDocumentGrid();
      this.initSearch();
      this.initLayout();
      
      setTimeout(function() {
        Ext.get('loading').remove();
        Ext.fly('loading-mask').fadeOut({
            remove: true
        });
      }, 250);
    },
    initLayout: function(){
      // --------------------------------------------
      // -- LAYOUT
      // --------------------------------------------
      var viewport = new Ext.Viewport({
        id: 'alfresco-browser-viewport',
        layout: 'border',
        items: [new Ext.BoxComponent({
          region: 'north',
          el: 'header',
          height: 35
        }), folderTree, {
          region: 'center',
          layout: 'border',
          border: false,
          margins: '5 5 5 0',
          items: [ itemsGrid, propsGrid ]
        }]
      });
    },
    initFolderTree: function(){
      // --------------------------------------------
      // -- NAVIGATION TREE
      // --------------------------------------------
      folderTree = new Ext.tree.TreePanel({
        id: 'folder-tree',
        region: 'west',
        collapsible: true,
        title: 'Browse Spaces',
        margins: '5 0 5 5',
        cmargins: '5 5 5 5',
        width: 240,
        minSize: 100,
        split: true,
        layout: 'fit',
        autoScroll: true,
        
        // tree specifics
        rootVisible: true,
        useArrows: true,
        trackMouseOver: false,

        loader: new Ext.tree.TreeLoader({
          dataUrl: AlfrescoBrowser.Settings['serviceTreeUrl'],
          requestMethod: 'GET'
        }),
        
        root: new Ext.tree.AsyncTreeNode({
          text: AlfrescoBrowser.Settings['homeText'],
          id: AlfrescoBrowser.Settings['homeRef'],
          expanded: true,
          listeners: {
            'load': function() {
              if (Ext.isEmpty(currentPath)) {
                this.fireEvent('click', this);
              }
              else {
                folderTree.expandPath(currentPath);
                folderTree.selectPath(currentPath);
              }
          }}
        }),
        
        listeners: {
          'click': function(node, e) {
            dataStore.baseParams = {node: node.id};
            dataStore.load({params:{start:0}});
            itemsGrid.setTitle(node.text);
            node.expand();
            Ext.getCmp('btn-add').enable();
        }},
        
        tools: [{
          id: 'refresh',
          on: {
            click: function(){
              var currentNode = folderTree.getSelectionModel().getSelectedNode();
              if (Ext.isEmpty(currentNode)) {
                currentNode = folderTree.root;
              }
              currentPath = currentNode.getPath();
              
              var baseParams = folderTree.loader.baseParams || {};
              folderTree.loader.baseParams['cache'] = 'all';
              folderTree.root.reload();
              folderTree.loader.baseParams = baseParams;
            }}
        }]
      });
    },
    initSearch: function(){
      // --------------------------------------------
      // -- SEARCH
      // --------------------------------------------
      Ext.app.SearchField = Ext.extend(Ext.form.TwinTriggerField, {
        initComponent : function(){
            Ext.app.SearchField.superclass.initComponent.call(this);
            this.on('specialkey', function(f, e){
                if(e.getKey() == e.ENTER){
                    this.onTrigger2Click();
                }
            }, this);
        },

        validationEvent:false,
        validateOnBlur:false,
        trigger1Class:'x-form-clear-trigger',
        trigger2Class:'x-form-search-trigger',
        hideTrigger1: true,
        hasSearch: false,
        paramName: 'query',
        
        onTrigger1Click : function(){
            if(this.hasSearch){
                this.el.dom.value = '';
                var node = folderTree.getSelectionModel().getSelectedNode();
                if (Ext.isEmpty(node)) {
                  node = folderTree.root;
                }
                this.store.baseParams = {node:node.id};
                this.store.load({params:{start:0}});
                this.triggers[0].hide();
                this.hasSearch = false;
                itemsGrid.setTitle(node.text);
            }
        },

        onTrigger2Click : function(){
            var v = this.getRawValue();
            if (v.length < 1) {
              this.onTrigger1Click();
              return;
            }
            if (v.length < 3) {
              return;
            }
            itemsGrid.setTitle('Search items: ' + Ext.util.Format.htmlEncode(v));
            Ext.getCmp('btn-add').disable();

            this.store.baseParams[this.paramName] = v;
            this.store.load({params:{start:0}});
            this.hasSearch = true;
            this.triggers[0].show();
        }
      });

      var search = new Ext.app.SearchField({
        region: 'west',
        store: dataStore,
        width: 320,
        emptyText: 'Search (minimum 3 characters)',
        applyTo: 'search'
      })
    },
    initDocumentGrid: function(){
      // --------------------------------------------
      // -- CONTENT GRID
      // --------------------------------------------
      var record = Ext.data.Record.create([
        {name: 'id'},   /* alfresco node id */
        {name: 'nid'},  /* drupal node id */
        {name: 'type'},
        {name: 'name'},
        {name: 'path'},
        {name: 'icon'},
        {name: 'author'},
        {name: 'creator'},
        {name: 'title'},
        {name: 'description'},
        {name: 'size'},
        {name: 'mimetype'},
        {name: 'created', type: 'date', dateFormat: 'Y-m-d H:i:s'},
        {name: 'modified', type: 'date', dateFormat: 'Y-m-d H:i:s'}
      ]);
      
      var reader = new Ext.data.JsonReader({
          totalProperty: 'total',
          root: 'rows',
          id: 'id'
      }, record);
      
      dataStore = new Ext.data.Store({
        proxy: new Ext.data.HttpProxy({
          url: AlfrescoBrowser.Settings['serviceGridUrl'],
          method: 'GET'
        }),
        reader: reader,
        baseParams: {node: AlfrescoBrowser.Settings['homeRef']},
        autoLoad: false,
        //remoteSort: true,
        sortInfo: {field: 'name', direction: 'ASC'},
        listeners: {
          load: {
            fn: function(){
              Ext.getCmp('btn-download').disable();
              Ext.getCmp('btn-delete').disable();
              Ext.getCmp('btn-open').disable();
              Ext.getCmp('btn-send').disable();
              Ext.getCmp('grid-details').disable();
            }
          }
        }
      });
      
      function renderName(value, p, record){
        var url = AlfrescoBrowser.Settings['modulePath'] + '/images/filetypes/' + record.data['icon'] + '.gif';
        return String.format('<span class="row-icon" style="background-image: url({1})" ext:qtip="{2}">{0}</span>', value, url, record.data['title']);
      }

      var columns = [
        {id: 'name', header: "Name", dataIndex: 'name', sortable: true, width: 200, renderer: renderName},
        {header: "Size", dataIndex: 'size', sortable: false, align: 'right', width: 80},
        {header: "Creator", dataIndex: 'creator', sortable: true, width: 100},
        {header: "Date created", dataIndex: 'created', width: 130, hidden: true, sortable: true, renderer: Ext.util.Format.dateRenderer('d-m-Y H:i:s')},
        {header: "Date modified", dataIndex: 'modified', width: 130, sortable: true, renderer: Ext.util.Format.dateRenderer('d-m-Y H:i:s')},
        {header: "Title", dataIndex: 'title', width: 200, sortable: true, hidden: true},
        {header: "Description", dataIndex: 'description', width: 200, sortable: false, hidden: true},
        {header: "Author", dataIndex: 'author', width: 100, sortable: true, hidden: true}
      ];
      
      var bar = new Ext.PagingToolbar({
        pageSize: AlfrescoBrowser.Settings['queryLimit'],
        store: dataStore,
        displayInfo: true,
        autoWidth: true,
        displayMsg: 'Displaying items {0} - {1} of {2}',
        emptyMsg: 'No items to display.',

        // override private event
        onClick: function(which){
          if (which == "refresh") {
            var o = {}, pn = this.paramNames;
            o[pn.start] = this.cursor;
            o[pn.limit] = this.pageSize;
            o['cache'] = 'node';
            if (this.fireEvent('beforechange', this, o) !== false) {
              this.store.load({params:o});
            }
            return;
          }
          Ext.PagingToolbar.prototype.onClick.call(this, which);
        }
      });

      itemsGrid = new Ext.grid.GridPanel({
        id: 'items-grid',
        ds: dataStore,
        columns: columns,
        autoExpandColumn: 'name',
        bbar: bar,
        sm: new Ext.grid.RowSelectionModel({
          singleSelect:true,
          listeners: {
            rowselect: function(sm, row, rec) {
              Ext.getCmp('btn-download').enable();
              Ext.getCmp('btn-delete').enable();
              Ext.getCmp('btn-open').enable();
              if (window.opener) {
                Ext.getCmp('btn-send').enable();
              }
              Ext.getCmp('grid-details').enable();
            },
            rowdeselect: function(sm, row, rec) {
            }
          }
        }),
        
        title: AlfrescoBrowser.Settings['homeText'],
        region: 'center',
        loadMask: true,
        
        listeners: {
          'rowclick': function(grid, dataIndex) {
            var dataRow = dataStore.getAt(dataIndex);
            propsGrid.setSource(dataRow.data);
          },
          'rowdblclick': function(grid, dataIndex) {
            if (window.opener) {
              AlfrescoBrowser.SendItem(grid);
            } else {
              AlfrescoBrowser.DownloadItem(grid, false);
            }
        }},
        
        viewConfig: {
          getRowClass: function(record, index) {
            return record.get('nid').length > 0 ? 'row-node-exists' : 'row-node-new';
          }
        },

        tbar: [{
          id: 'btn-add',
          text: 'Add',
          tooltip: 'Add content to this space.',
          iconCls: 'upload',
          disabled: true,
          handler: function() {
            AlfrescoBrowser.AddItem(folderTree, dataStore);
          }
        }, '-', {
          id: 'btn-delete',
          text: 'Delete',
          tooltip: 'Delete selected content.',
          iconCls: 'delete',
          disabled: true,
          handler: function() {
            AlfrescoBrowser.DeleteItem(itemsGrid);
          }
        }, '-', {
          id: 'btn-download',
          text: 'Download',
          tooltip: 'Download selected item.',
          iconCls: 'download',
          disabled: true,
          handler: function() {
            AlfrescoBrowser.DownloadItem(itemsGrid, true);
          }
        }, '-', {
          id: 'btn-open',
          text: 'View',
          tooltip: 'View selected item in new window.',
          iconCls: 'view',
          disabled: true,
          handler: function() {
            AlfrescoBrowser.ViewItem(itemsGrid);
          }
        },'-',{
          id: 'btn-send',
          text: 'Send to Drupal',
          tooltip: 'Send selected item to Drupal.',
          iconCls: 'drupal',
          disabled: true,
          handler: function() {
            AlfrescoBrowser.SendItem(itemsGrid);
          }
        }, /*'-',*/ {
          id: 'grid-details',
          text: 'Properties',
          tooltip: 'View node properties.',
          iconCls: 'details',
          disabled: true,
          hidden: true,
          enableToggle: true,
          toggleHandler: function(item, pressed){
            if (pressed) {
              propsGrid.expand();
            } else {
              propsGrid.collapse();
            }
          }
        }],

        tools: [{
          id: 'refresh',
          qtip: 'Clear content and search cache.',
          on: {
            click: function(){
              var o = {start: 0, cache: 'all'};
              dataStore.load({params:o});
            }}
        }]
      });
      
      propsGrid = new Ext.grid.PropertyGrid({
        title: 'Properties',
        region: 'south',
        margins: '0 0 0 0',
        cmargins: '5 0 0 0',
        height: 150,
        minSize: 80,
        maxSize: 300,
        collapsible: true,
        collapsed: true,
        collapseMode: 'mini',
        split: true,
        hideCollapseTool: true,
        hideHeaders: true,
        listeners: {
        'validateedit': function(e) {
          e.cancel = true;
        },
        'expand': function(p) {
          Ext.getCmp('grid-details').toggle(true);
        },
        'collapse': function(p) {
          Ext.getCmp('grid-details').toggle(false);
        }}
      });
    }
  };
}();
Ext.onReady(AlfrescoBrowser.App.init, AlfrescoBrowser.App);