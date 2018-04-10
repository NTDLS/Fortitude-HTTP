<!--#include virtual="/@PageTop.Html"-->

<h2>Simple Scripting Engine Test</h2>
<hr />


<%
If(!Form.Value.IsDefined("Posted"))
	Form.Value.Define("Username", "Administrator")
	Form.Value.Define("Password", "Default")
End If
%>

<form method="Post" action="?">
    <table border="0" cellpadding="2" cellspacing="0" width="200">
        <tr>
            <td class="DefaultTableHeader" colspan="2"><center><b>Test Input</b></center></td>
        </tr>
        <tr>
            <td class="DefaultTable" width="10%">Username :</td>
            <td class="DefaultTable"><input type="input" name="Username" size="17" value="<%=Form.Username%>"></td>
        </tr>
        <tr>
            <td class="DefaultTable" width="10%">Password :</td>
            <td class="DefaultTable"><input type="password" name="Password" size="17" value="<%=Form.Password%>"></td>
        </tr>
        <tr>
            <td class="DefaultTable" width="10%"><input type="hidden" name="Posted" value="Yes">&nbsp;</td>
            <td class="DefaultTable"><input type="submit" value= " Login "></td>
        </tr>
    </table>
<form>

<%If(Form.Value.IsDefined("Posted"))%>
	<br /><br />
    <table border="0" cellpadding="2" cellspacing="0" width="200">
        <tr>
            <td class="DefaultTableHeader" colspan="2"><center><b>Test Input Was</b></center></td>
        </tr>
        <tr>
            <td class="DefaultTable" width="10%">Username :</td>
            <td class="DefaultTable"><%=Form.Username%></td>
        </tr>
        <tr>
            <td class="DefaultTable" width="10%">Password :</td>
            <td class="DefaultTable"><%=Form.Password%></td>
        </tr>
    </table>
<%End If%>

<br /><br />

<!--#include virtual="/@PageBottom.Html"-->
