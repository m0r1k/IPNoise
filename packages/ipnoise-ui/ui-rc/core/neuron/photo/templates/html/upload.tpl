<form enctype="multipart/form-data" method="post" id="updatePhoto1"
    action="/api?object_id=<perl>return getCurNeuron()->getId();</perl>&method=upload">
    <table>
        <tr>
            <td>Файл для загрузки:</td>
            <td>
                <input type="file" name="file" size="34" class="multi max-1"></input>
            </td>
        </tr>
        <tr>
            <td>
                <input type="submit" value="Сохранить"></input>
            </td>
            <td></td>
        </tr>
    </table>
</form>

