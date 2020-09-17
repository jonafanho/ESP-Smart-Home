const PORT_COUNT = 4;

function setup() {
	"use strict";

	class MainScreen extends React.Component {

		constructor(props) {
			super(props);
			this.editOrSave = this.editOrSave.bind(this);
			this.state = {edit: false};
		}

		editOrSave(event) {
			const isEdit = this.state.edit;
			if (isEdit) {
				fetch(`/settings?`, {
					method: "POST",
					body: "",
					headers: {
						"Content-type": "application/json; charset=UTF-8"
					}
				}).then(response => response.json()).then(data => {
				}).catch(error => {
				});
			}
			this.setState({edit: !isEdit});
		}

		render() {
			return (
				<div>
					<h1>Smart Home Device Configuration</h1>
					<table>
						<tbody>
						{[...Array(4)].map((u, i) => <Port key={i} edit={this.state.edit} index={i}/>)}
						</tbody>
					</table>
					<input
						className="input_button"
						type="submit"
						value={this.state.edit ? "Save" : "Edit"}
						onClick={this.editOrSave}
					/>
				</div>
			);
		}
	}

	class Port extends React.Component {

		constructor(props) {
			super(props);
		}

		render() {
			const {index, edit} = this.props;
			return (
				<tr>
					<td><h2>Port {index + 1}</h2></td>
				</tr>
			);
		}
	}

	if (window.location.pathname !== "/") {
		window.location.pathname = "/";
	}
	ReactDOM.render(<MainScreen/>, document.querySelector("#react-root"));
}

setup();