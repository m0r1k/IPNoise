<script>
    $(document).ready(function(){
        $( "#accordion_bookmarks" ).accordion({
            active:         false,
            collapsible:    true,
            heightStyle:    'content'
        });
        $( "#accordion_object" ).accordion({
            // active:         true,
            collapsible:    true,
            heightStyle:    'content'
        });
        $( "#accordion_context" ).accordion({
            active:         false,
            collapsible:    true,
            heightStyle:    'content'
        });
        $( "#accordion_object_new" ).accordion({
            active:         false,
            collapsible:    true,
            heightStyle:    'content'
        });

        $('#admin_tabs-3')
            .find('select#neuron_type :contains("Elementary")')
            .attr('selected', 'selected');

        $('#admin_tabs-3').find('#neuron_type').change(function(){
            var val = $("select#neuron_type").val();
            if (2 == val){
                $('#admin_tabs-3').find('tr.user').css({
                    'display':  ''
                });
            } else {
                $('#admin_tabs-3').find('tr.user').css({
                    'display':  'none'
                });
            }
        });

        $('#addToBookmarks').submit(function(){
            addToBookmarks(getCurNeuronId());
            return false;
        });

        $('#createObject').submit(function(){
            createObject(
                getCurNeuronId(),
                {
                    'type': $('#createObject').find('#object_type option:selected').val(),
                }
            );
            return false;
        });

        $('.neigh_add').click(function (ev){
            processNeigh('add', ev);
        });
        $('.neigh_unlink').click(function (ev){
            processNeigh('unlink', ev);
        });
        $('.neigh_delete').click(function (ev){
            processNeigh('delete', ev);
        });
        $('.bookmark_add').click(function (ev){
            processNeigh('bookmark_add', ev);
        });
        $('.bookmark_del').click(function (ev){
            processNeigh('bookmark_del', ev);
        });
    });
</script>

<perl>
    my $ret = '';
    $ret .= processCurTemplate("admin/head.tpl");
    return $ret;
</perl>

