<div style="width: 100%;"
    id="<perl>return getCurNeuronId()</perl>"
    class="neuron neuronDream">

    <script>
        $(document).ready(function(){
            // setup neuron
            var neuron = NeuronDreamClass();
            neuron.setId("<perl>return getCurNeuronId()</perl>");
            neuron.load();
        });
    </script>

    <center>
    <h1>Редактирование сна</h1>
    </center>

    <div class="neuron_dream_edit">
        <table>
            <tr>
                <td>Время начала и окончания</td>
                <td>
                    <input type="text"></input>
                    <input type="text"></input>
                </td>
            </tr>
            <tr>
                <td>Общее описание</td>
                <td><textarea>вуги</textarea></td>
            </tr>
            <tr>
                <td>Выявленные ошибки</td>
                <td><input type="text"></input></td>
            </tr>
            <tr>
                <td>
                    <input type="button" value="Сохранить"
                        onclick="uiGetNeuron(this).save(this, event);"/>
                </td>
                <td></td>
            </tr>
        </table>
    </div>

</div>

