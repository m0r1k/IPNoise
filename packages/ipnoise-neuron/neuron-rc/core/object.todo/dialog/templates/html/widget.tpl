<div style="width: 100%;"
    id="<perl>return getCurObjectId()</perl>"
    class="object">

    <script>
    </script>

    <table class="object_dialog_widget">
        <tr>
             <td>
                  <div class="object_dialog_messages">
                  <perl>
                      my $ret       = '';
                      my $object    = getCurObject();
                      my $msgs      = $object->getNeighsByType('dialog_msg');

                      foreach my $msg (@$msgs){
                          $ret .= $msg->renderEmbedded('widget');
                      }

                      return $ret;
                  </perl>
                  </div>
            </td>
        </tr>
        <tr>
            <td>
                <table class="object_dialog_controls">
                    <tr>
                        <td>
                            <textarea class="object_dialog_controls_input"></textarea>
                        </td>
                    </tr>
                    <tr>
                        <td>
                            <input type="button" value="Отправить" class="object_dialog_controls_submit"/>
                        </td>
                    </tr>
                </table>
            </td>
        </tr>
    </table>
</div>

